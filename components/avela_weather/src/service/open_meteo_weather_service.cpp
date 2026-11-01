#include "avela_weather/service/open_meteo_weather_service.hpp"

#include <cmath>
#include <optional>

#include "avela_hal/memory/heap_watch.hpp"
#include "avela_settings/settings.hpp"
#include "avela_weather/service/open_meteo.hpp"
#include "esp_crt_bundle.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "sdkconfig.h"

namespace avela {
namespace {

constexpr const char* log_tag = "weather";
constexpr int64_t microseconds_per_millisecond = 1000;
constexpr int64_t milliseconds_per_minute = 60'000;
constexpr int64_t update_interval_ms = CONFIG_AVELA_WEATHER_REFRESH_MINUTES * milliseconds_per_minute;
constexpr int64_t retry_after_failure_ms = milliseconds_per_minute;
constexpr TickType_t idle_wait = pdMS_TO_TICKS(1000);
constexpr int http_timeout_ms = 10'000;
constexpr int http_ok = 200;
constexpr std::size_t url_capacity = 384;
using Url = FixedString<url_capacity>;

int64_t now_ms()
{
    return esp_timer_get_time() / microseconds_per_millisecond;
}

// Holds a FreeRTOS mutex for one scope.
class LockGuard {
public:
    explicit LockGuard(SemaphoreHandle_t lock) : m_lock_(lock) { xSemaphoreTake(m_lock_, portMAX_DELAY); }
    LockGuard(const LockGuard&) = delete;
    LockGuard& operator=(const LockGuard&) = delete;
    LockGuard(LockGuard&&) = delete;
    LockGuard& operator=(LockGuard&&) = delete;
    ~LockGuard() { xSemaphoreGive(m_lock_); }

private:
    SemaphoreHandle_t m_lock_;
};

// The place as a URL query value: letters, digits and -_.~ as they are, every other byte as %XX.
void append_query_value(Url& url, std::string_view text)
{
    constexpr std::string_view hex_digits = "0123456789ABCDEF";
    constexpr unsigned high_nibble_shift = 4;
    constexpr unsigned nibble_mask = 0x0F;
    for (const char character : text) {
        const bool unreserved = (character >= 'A' && character <= 'Z') || (character >= 'a' && character <= 'z') ||
                                (character >= '0' && character <= '9') || character == '-' || character == '_' ||
                                character == '.' || character == '~';
        if (unreserved) {
            url.append(std::string_view{&character, 1});
            continue;
        }
        const auto byte = static_cast<unsigned char>(character);
        const std::array<char, 3> escaped{'%', hex_digits[(byte >> high_nibble_shift) & nibble_mask],
                                          hex_digits[byte & nibble_mask]};
        url.append(std::string_view{escaped.data(), escaped.size()});
    }
}

// Micro-degrees as a decimal number of degrees: 52229770 -> "52.229770".
void append_degrees(Url& url, int32_t micro_degrees)
{
    constexpr std::size_t decimals = 6;
    if (micro_degrees < 0) {
        url.append("-");
    }
    const auto magnitude = static_cast<uint32_t>(std::abs(static_cast<int64_t>(micro_degrees)));
    const auto per_degree = static_cast<uint32_t>(WeatherSettings::micro_degrees_per_degree);
    url.append_number(magnitude / per_degree).append(".").append_number(magnitude % per_degree, decimals);
}

int whole(double value)
{
    return static_cast<int>(std::lround(value));
}

int32_t micro_degrees(double degrees)
{
    return static_cast<int32_t>(std::llround(degrees * WeatherSettings::micro_degrees_per_degree));
}

}  // namespace

//--------------------------------------------------------------------------------------------------

OpenMeteoWeatherService::OpenMeteoWeatherService(const NetworkModel& network)
    : m_network_(network), m_lock_(xSemaphoreCreateMutexStatic(&m_lock_storage_))
{}

//--------------------------------------------------------------------------------------------------

void OpenMeteoWeatherService::init()
{
    constexpr UBaseType_t priority = 1;  // below the network stack; the main loop keeps drawing meanwhile
    xTaskCreateStatic(&OpenMeteoWeatherService::run, task_name, m_stack_.size(), this, priority, m_stack_.data(),
                      &m_task_);
}

//--------------------------------------------------------------------------------------------------

void OpenMeteoWeatherService::update(WeatherModel& model)
{
    m_joined_.store(m_network_.m_status == WifiStatus::connected);
    while (m_requests_.take()) {
        m_update_requested_.store(true);
    }
    const uint32_t revision = m_published_revision_.load();
    if (revision != m_copied_revision_) {
        const LockGuard guard{m_lock_};
        model.m_report = m_published_;
        m_copied_revision_ = revision;
    }
}

//--------------------------------------------------------------------------------------------------

void OpenMeteoWeatherService::run(void* self)
{
    static_cast<OpenMeteoWeatherService*>(self)->run();
}

//--------------------------------------------------------------------------------------------------

void OpenMeteoWeatherService::run()
{
    publish();  // not configured, until the settings say otherwise
    while (true) {
        const bool report_reset = follow_settings();
        const WeatherStatus waiting = waiting_status();
        if (waiting != WeatherStatus::ready) {
            // a reset report goes out even in the same status: the screen still shows the old values
            if (report_reset || m_report_.m_status != waiting) {
                m_report_.m_status = waiting;
                publish();
            }
        } else if (update_due()) {
            update_now();
            continue;
        }
        vTaskDelay(idle_wait);
    }
}

//--------------------------------------------------------------------------------------------------

bool OpenMeteoWeatherService::follow_settings()
{
    const uint32_t revision = Settings::revision<WeatherSettings>();
    if (m_settings_read_ && revision == m_settings_revision_) {
        return false;
    }
    m_settings_read_ = true;
    m_settings_revision_ = revision;
    m_has_settings_ = load_settings(m_settings_);
    m_next_update_ms_ = now_ms();  // new settings: update at once
    return match_report(m_report_, m_settings_);
}

//--------------------------------------------------------------------------------------------------

WeatherStatus OpenMeteoWeatherService::waiting_status() const
{
    if (!m_has_settings_ || m_settings_.m_place.view().empty()) {
        return WeatherStatus::not_configured;
    }
    if (!m_joined_.load()) {
        return WeatherStatus::waiting_for_network;
    }
    return WeatherStatus::ready;
}

//--------------------------------------------------------------------------------------------------

bool OpenMeteoWeatherService::update_due()
{
    // asked for, or just joined the network: don't wait for the next interval
    if (m_update_requested_.exchange(false) || m_report_.m_status == WeatherStatus::waiting_for_network) {
        m_next_update_ms_ = now_ms();
    }
    return now_ms() >= m_next_update_ms_;
}

//--------------------------------------------------------------------------------------------------

void OpenMeteoWeatherService::update_now()
{
    m_report_.m_status = WeatherStatus::loading;
    publish();
    if (!m_settings_.m_located) {
        WeatherSettings looked_up = m_settings_;
        if (locate(looked_up) && store_location(looked_up)) {
            return;  // the settings changed: follow_settings() takes them and updates at once
        }
    }
    const bool updated = m_settings_.m_located && fetch(m_settings_);
    m_next_update_ms_ = now_ms() + (updated ? update_interval_ms : retry_after_failure_ms);
    publish();
}

//--------------------------------------------------------------------------------------------------

bool OpenMeteoWeatherService::store_location(const WeatherSettings& looked_up)
{
    const bool stored =
        Settings::update<WeatherSettings>([&](WeatherSettings& saved) { return take_location(saved, looked_up); });
    if (stored || Settings::revision<WeatherSettings>() != m_settings_revision_) {
        return true;
    }
    ESP_LOGW(log_tag, "couldn't store the place's coordinates; looking it up again after a restart");
    m_settings_ = looked_up;
    return false;
}

//--------------------------------------------------------------------------------------------------

bool OpenMeteoWeatherService::load_settings(WeatherSettings& settings)
{
    settings = WeatherSettings{};
    return Settings::load(settings);
}

//--------------------------------------------------------------------------------------------------

bool OpenMeteoWeatherService::locate(WeatherSettings& settings)
{
    Url url;
    url.append("https://geocoding-api.open-meteo.com/v1/search?count=1&language=en&format=json&name=");
    append_query_value(url, settings.m_place.view());
    std::string_view body;
    if (!get(url.c_str(), body)) {
        m_report_.m_status = WeatherStatus::failed;
        return false;
    }
    std::array<std::string_view, 1> results{};
    const std::optional<double> latitude = open_meteo::elements(open_meteo::value_of(body, "results"), results) == 1
                                               ? open_meteo::number(open_meteo::value_of(results[0], "latitude"))
                                               : std::nullopt;
    const std::optional<double> longitude =
        latitude ? open_meteo::number(open_meteo::value_of(results[0], "longitude")) : std::nullopt;
    if (!latitude || !longitude) {
        ESP_LOGW(log_tag, "no place called \"%s\"", settings.m_place.c_str());
        m_report_.m_status = WeatherStatus::place_not_found;
        return false;
    }
    settings.m_located = true;
    settings.m_latitude_micro_degrees = micro_degrees(*latitude);
    settings.m_longitude_micro_degrees = micro_degrees(*longitude);
    settings.m_located_name.clear().append(open_meteo::string(open_meteo::value_of(results[0], "name")).value_or(""));
    if (const auto country = open_meteo::string(open_meteo::value_of(results[0], "country"))) {
        settings.m_located_name.append(", ").append(*country);
    }
    ESP_LOGI(log_tag, "%s is %s", settings.m_place.c_str(), settings.m_located_name.c_str());
    return true;
}

//--------------------------------------------------------------------------------------------------

bool OpenMeteoWeatherService::fetch(const WeatherSettings& settings)
{
    Url url;
    url.append("https://api.open-meteo.com/v1/forecast?latitude=");
    append_degrees(url, settings.m_latitude_micro_degrees);
    url.append("&longitude=");
    append_degrees(url, settings.m_longitude_micro_degrees);
    url.append("&current=temperature_2m,apparent_temperature,relative_humidity_2m,weather_code,wind_speed_10m,is_day"
               "&daily=weather_code,temperature_2m_max,temperature_2m_min&timezone=auto&forecast_days=5");
    if (settings.m_units == MeasurementSystem::imperial) {
        url.append("&temperature_unit=fahrenheit&wind_speed_unit=mph");
    }
    std::string_view body;
    if (!get(url.c_str(), body)) {
        m_report_.m_status = WeatherStatus::failed;
        return false;
    }

    using open_meteo::number;
    using open_meteo::value_of;
    const std::string_view current = value_of(body, "current");
    const auto temperature = number(value_of(current, "temperature_2m"));
    const auto feels_like = number(value_of(current, "apparent_temperature"));
    const auto humidity = number(value_of(current, "relative_humidity_2m"));
    const auto code = number(value_of(current, "weather_code"));
    const auto wind = number(value_of(current, "wind_speed_10m"));
    const auto day = number(value_of(current, "is_day"));

    constexpr std::size_t days = WeatherReport::forecast_days + 1;  // today and the days after
    const std::string_view daily = value_of(body, "daily");
    std::array<std::string_view, days> dates{};
    std::array<std::string_view, days> codes{};
    std::array<std::string_view, days> highs{};
    std::array<std::string_view, days> lows{};
    const bool complete = temperature && feels_like && humidity && code && wind && day &&
                          open_meteo::elements(value_of(daily, "time"), dates) == days &&
                          open_meteo::elements(value_of(daily, "weather_code"), codes) == days &&
                          open_meteo::elements(value_of(daily, "temperature_2m_max"), highs) == days &&
                          open_meteo::elements(value_of(daily, "temperature_2m_min"), lows) == days;
    if (!complete) {
        ESP_LOGW(log_tag, "unexpected answer from Open-Meteo");
        m_report_.m_status = WeatherStatus::failed;
        return false;
    }

    WeatherReport& report = m_report_;
    report.m_temperature = whole(*temperature);
    report.m_feels_like = whole(*feels_like);
    report.m_humidity_percent = whole(*humidity);
    report.m_condition = open_meteo::condition_of(whole(*code));
    report.m_wind_speed = whole(*wind);
    report.m_daytime = whole(*day) != 0;
    report.m_high = whole(number(highs[0]).value_or(0.0));
    report.m_low = whole(number(lows[0]).value_or(0.0));
    for (std::size_t index = 0; index < report.m_forecast.size(); ++index) {
        const std::size_t day_index = index + 1;
        const auto date = open_meteo::string(dates.at(day_index)).value_or("");
        report.m_forecast.at(index) = ForecastDay{
            .m_weekday = open_meteo::weekday_of(date).value_or(0),
            .m_condition = open_meteo::condition_of(whole(number(codes.at(day_index)).value_or(0.0))),
            .m_high = whole(number(highs.at(day_index)).value_or(0.0)),
            .m_low = whole(number(lows.at(day_index)).value_or(0.0)),
        };
    }
    report.m_has_values = true;
    report.m_status = WeatherStatus::ready;
    return true;
}

//--------------------------------------------------------------------------------------------------

bool OpenMeteoWeatherService::get(const char* url, std::string_view& body)
{
    const HeapWatch heap_watch;  // what the request costs: logged once it is done
    esp_http_client_config_t config{};
    config.url = url;
    config.timeout_ms = http_timeout_ms;
    config.crt_bundle_attach = esp_crt_bundle_attach;
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == nullptr) {
        return false;
    }
    std::size_t length = 0;
    bool complete = false;
    if (esp_http_client_open(client, 0) == ESP_OK && esp_http_client_fetch_headers(client) >= 0 &&
        esp_http_client_get_status_code(client) == http_ok) {
        const std::size_t room = m_response_.size() - 1;
        while (length < room) {
            const int read = esp_http_client_read(client, &m_response_.at(length), static_cast<int>(room - length));
            if (read <= 0) {
                complete = read == 0 && esp_http_client_is_complete_data_received(client);
                break;
            }
            length += static_cast<std::size_t>(read);
        }
    }
    if (!complete) {
        ESP_LOGW(log_tag, "request failed (or answer over %zu bytes): %s", m_response_.size() - 1, url);
    }
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    heap_watch.log(log_tag, "request");
    body = std::string_view{m_response_.data(), length};
    return complete;
}

//--------------------------------------------------------------------------------------------------

void OpenMeteoWeatherService::publish()
{
    ++m_report_.m_revision;
    {
        const LockGuard guard{m_lock_};
        m_published_ = m_report_;
    }
    m_published_revision_.store(m_report_.m_revision);
}

}  // namespace avela
