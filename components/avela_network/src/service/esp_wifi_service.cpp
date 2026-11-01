#include "avela_network/service/esp_wifi_service.hpp"

#include <algorithm>
#include <optional>
#include <span>
#include <string_view>

#include "avela_clock/system_clock.hpp"
#include "avela_settings/settings.hpp"
#include "esp_log.h"
#include "esp_netif_sntp.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "mdns.h"

namespace avela {
namespace {

constexpr const char* log_tag = "network";
constexpr const char* time_server = "pool.ntp.org";
constexpr const char* mdns_instance_name = "Avela radio";
constexpr uint16_t http_port = 80;
constexpr uint8_t setup_network_max_clients = 4;
constexpr int64_t microseconds_per_millisecond = 1000;

using AddressText = FixedString<NetworkModel::address_capacity>;

//--------------------------------------------------------------------------------------------------

uint32_t now_ms()
{
    return static_cast<uint32_t>(esp_timer_get_time() / microseconds_per_millisecond);
}

//--------------------------------------------------------------------------------------------------

// Whether `deadline` has come, on a wrapping millisecond clock.
bool reached(uint32_t now, uint32_t deadline)
{
    return static_cast<int32_t>(now - deadline) >= 0;
}

//--------------------------------------------------------------------------------------------------

AddressText address_text(uint32_t address)
{
    const esp_ip4_addr_t ip{.addr = address};
    std::array<char, NetworkModel::address_capacity> characters{};
    esp_ip4addr_ntoa(&ip, characters.data(), static_cast<int>(characters.size()));
    AddressText text;
    text.append(characters.data());
    return text;
}

//--------------------------------------------------------------------------------------------------

// Copies text into a driver field (fixed byte array), cut to its size; the rest stays zero.
void copy_to_field(std::string_view text, std::span<uint8_t> field)
{
    std::transform(text.begin(),
                   std::next(text.begin(), static_cast<std::ptrdiff_t>(std::min(text.size(), field.size()))),
                   field.begin(), [](char character) { return static_cast<uint8_t>(character); });
}

//--------------------------------------------------------------------------------------------------

// ESP-IDF's SNTP has set the system clock (before calling this): the clock only needs to know.
void on_time_synchronised(timeval* /*time*/)
{
    SystemClock::mark_synchronised();
    ESP_LOGI(log_tag, "clock set by SNTP");
}

//--------------------------------------------------------------------------------------------------

void log_if_failed(esp_err_t result, const char* what)
{
    if (result != ESP_OK) {
        ESP_LOGW(log_tag, "%s: %s", what, esp_err_to_name(result));
    }
}

}  // namespace

//--------------------------------------------------------------------------------------------------

void EspWifiService::init()
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    m_station_ = esp_netif_create_default_wifi_sta();
    m_access_point_ = esp_netif_create_default_wifi_ap();
    m_host_name_ = saved_host_name();
    log_if_failed(esp_netif_set_hostname(m_station_, m_host_name_.m_name.c_str()), "host name");

    const wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&config));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));  // the saved network is our Settings record
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &EspWifiService::on_event, this));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &EspWifiService::on_event, this));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_LOST_IP, &EspWifiService::on_event, this));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    // Modem sleep (the default) wakes the radio only at the router's beacons: an audio stream then arrives in bursts
    // and runs dry. A mains-powered radio has no reason to save power on Wi-Fi.
    log_if_failed(esp_wifi_set_ps(WIFI_PS_NONE), "power save off");

    m_dns_.init();

    // mDNS follows the interfaces by itself: http://<host name>.local on the home network
    log_if_failed(mdns_init(), "mDNS");
    log_if_failed(mdns_hostname_set(m_host_name_.m_name.c_str()), "mDNS host name");
    log_if_failed(mdns_instance_name_set(mdns_instance_name), "mDNS instance name");
    log_if_failed(mdns_service_add(nullptr, "_http", "_tcp", http_port, nullptr, 0), "mDNS service");
}

//--------------------------------------------------------------------------------------------------

void EspWifiService::update(NetworkModel& model)
{
    const uint32_t now = now_ms();
    while (const std::optional<SetupNetworkRequest> request = m_requests_.take()) {
        m_setup_network_wanted_ = request->m_open;
    }
    follow_settings(model, now);
    follow_setup_network(model);
    follow_events(model, now);
    start_next_activity(model, now);
    report_signal(model, now);
}

//--------------------------------------------------------------------------------------------------

void EspWifiService::follow_settings(NetworkModel& model, uint32_t now_ms)
{
    const uint32_t revision = Settings::revision<WifiCredentials>();
    if (m_credentials_revision_ == revision) {
        return;
    }
    const bool saved_on_setup_page = m_credentials_revision_.has_value();  // not the first load at start
    m_credentials_revision_ = revision;
    model.m_host_name = m_host_name_.m_name;
    if (saved_on_setup_page) {
        m_setup_network_wanted_ = false;
        close_setup_network(model);
    }
    WifiCredentials loaded{};
    m_has_credentials_ = Settings::load(loaded) && !loaded.m_ssid.empty();
    m_credentials_ = m_has_credentials_ ? loaded : WifiCredentials{};

    if (m_activity_ == Activity::joining || model.m_status == WifiStatus::connected) {
        log_if_failed(esp_wifi_disconnect(), "disconnect");
        m_address_.store(0);  // the old network's address must not count as joining the new one
        m_activity_ = Activity::idle;
    }
    model.m_ssid = m_credentials_.m_ssid;
    model.m_address.clear();
    model.m_signal_dbm = 0;
    ++model.m_revision;
    if (!m_has_credentials_) {
        ESP_LOGI(log_tag, "no network saved");
        model.m_status = WifiStatus::not_configured;
        return;
    }
    wifi_config_t config{};
    copy_to_field(m_credentials_.m_ssid.view(), config.sta.ssid);
    copy_to_field(m_credentials_.m_password.view(), config.sta.password);
    config.sta.threshold.authmode = m_credentials_.m_password.empty() ? WIFI_AUTH_OPEN : WIFI_AUTH_WPA2_PSK;
    log_if_failed(esp_wifi_set_config(WIFI_IF_STA, &config), "station config");
    ESP_LOGI(log_tag, "joining %s", m_credentials_.m_ssid.c_str());
    model.m_status = WifiStatus::connecting;
    m_next_attempt_ms_ = now_ms;
}

//--------------------------------------------------------------------------------------------------

void EspWifiService::follow_setup_network(NetworkModel& model)
{
    if (m_setup_network_wanted_) {
        open_setup_network(model);
    } else {
        close_setup_network(model);
    }
}

//--------------------------------------------------------------------------------------------------

void EspWifiService::follow_events(NetworkModel& model, uint32_t now_ms)
{
    const uint32_t disconnects = m_disconnects_.load();
    const bool dropped = disconnects != m_seen_disconnects_;
    m_seen_disconnects_ = disconnects;
    const uint32_t address = m_address_.load();

    if (m_has_credentials_ && address != 0 && model.m_status != WifiStatus::connected) {
        m_activity_ = Activity::idle;
        model.m_status = WifiStatus::connected;
        model.m_address = address_text(address);
        ++model.m_revision;
        ESP_LOGI(log_tag, "joined %s as %s", m_credentials_.m_ssid.c_str(), model.m_address.c_str());
        start_internet_services();
        m_next_signal_ms_ = now_ms;
    } else if (model.m_status == WifiStatus::connected && (dropped || address == 0)) {
        ESP_LOGI(log_tag, "connection lost: rejoining");
        model.m_status = WifiStatus::connecting;  // ESP-IDF doesn't rejoin by itself
        model.m_address.clear();
        model.m_signal_dbm = 0;
        ++model.m_revision;
        m_next_attempt_ms_ = now_ms;
    } else if (dropped && m_activity_ == Activity::joining) {
        m_activity_ = Activity::idle;  // the attempt failed
        m_next_attempt_ms_ = now_ms + retry_delay_ms;
    }

    if (m_activity_ == Activity::joining && now_ms - m_activity_since_ms_ >= attempt_timeout_ms) {
        log_if_failed(esp_wifi_disconnect(), "disconnect");
        m_activity_ = Activity::idle;
        m_next_attempt_ms_ = now_ms + retry_delay_ms;
    }
}

//--------------------------------------------------------------------------------------------------

void EspWifiService::start_next_activity(const NetworkModel& model, uint32_t now_ms)
{
    if (m_activity_ != Activity::idle) {
        return;
    }
    const bool join_due = m_has_credentials_ && model.m_status != WifiStatus::connected &&
                          !model.m_setup_network_active && reached(now_ms, m_next_attempt_ms_);
    if (join_due) {
        if (const esp_err_t result = esp_wifi_connect(); result != ESP_OK) {
            log_if_failed(result, "join");
            m_next_attempt_ms_ = now_ms + retry_delay_ms;
            return;
        }
        m_activity_ = Activity::joining;
        m_activity_since_ms_ = now_ms;
    }
}

//--------------------------------------------------------------------------------------------------

void EspWifiService::open_setup_network(NetworkModel& model)
{
    if (model.m_setup_network_active) {
        return;
    }
    if (m_activity_ == Activity::joining) {
        log_if_failed(esp_wifi_disconnect(), "disconnect");  // no join attempts while the setup network is up
        m_activity_ = Activity::idle;
    }
    ESP_LOGI(log_tag, "setup network on");
    log_if_failed(esp_wifi_set_mode(WIFI_MODE_APSTA), "setup network mode");
    wifi_config_t config{};
    constexpr std::string_view name = CONFIG_AVELA_SETUP_NETWORK_NAME;
    constexpr std::string_view password = CONFIG_AVELA_SETUP_NETWORK_PASSWORD;
    static_assert(password.size() >= WifiCredentials::minimum_password_length &&
                      password.size() < sizeof(config.ap.password),
                  "the setup network's WPA2 password needs 8 to 63 characters");
    copy_to_field(name, config.ap.ssid);
    copy_to_field(password, config.ap.password);
    config.ap.ssid_len = static_cast<uint8_t>(name.size());
    config.ap.authmode = WIFI_AUTH_WPA2_PSK;
    config.ap.max_connection = setup_network_max_clients;
    log_if_failed(esp_wifi_set_config(WIFI_IF_AP, &config), "setup network config");

    esp_netif_ip_info_t info{};
    if (esp_netif_get_ip_info(m_access_point_, &info) == ESP_OK) {
        m_dns_.enable(info.ip.addr);
        model.m_setup_address = address_text(info.ip.addr);
    }
    model.m_setup_network_active = true;
    ++model.m_revision;
}

//--------------------------------------------------------------------------------------------------

void EspWifiService::close_setup_network(NetworkModel& model)
{
    if (!model.m_setup_network_active) {
        return;
    }
    ESP_LOGI(log_tag, "setup network off");
    m_dns_.disable();
    log_if_failed(esp_wifi_set_mode(WIFI_MODE_STA), "station mode");
    model.m_setup_network_active = false;
    model.m_setup_address.clear();
    ++model.m_revision;
}

//--------------------------------------------------------------------------------------------------

void EspWifiService::report_signal(NetworkModel& model, uint32_t now_ms)
{
    if (model.m_status != WifiStatus::connected || !reached(now_ms, m_next_signal_ms_)) {
        return;
    }
    m_next_signal_ms_ = now_ms + signal_interval_ms;
    wifi_ap_record_t access_point{};
    if (esp_wifi_sta_get_ap_info(&access_point) != ESP_OK) {
        return;
    }
    // NOLINTNEXTLINE(bugprone-signed-char-misuse,cert-str34-c): rssi is an int8_t number of dBm, not a character
    const int signal_dbm = access_point.rssi;
    if (signal_dbm != model.m_signal_dbm) {
        model.m_signal_dbm = signal_dbm;
        ++model.m_revision;
    }
}

//--------------------------------------------------------------------------------------------------

void EspWifiService::start_internet_services()
{
    if (m_internet_services_started_) {
        return;
    }
    m_internet_services_started_ = true;
    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG(time_server);
    config.sync_cb = &on_time_synchronised;
    log_if_failed(esp_netif_sntp_init(&config), "SNTP");
}

//--------------------------------------------------------------------------------------------------

void EspWifiService::on_event(void* self, esp_event_base_t base, int32_t id, void* data)
{
    auto& service = *static_cast<EspWifiService*>(self);
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        service.m_address_.store(0);
        service.m_disconnects_.fetch_add(1);
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        service.m_address_.store(static_cast<const ip_event_got_ip_t*>(data)->ip_info.ip.addr);
    } else if (base == IP_EVENT && id == IP_EVENT_STA_LOST_IP) {
        service.m_address_.store(0);
    }
}

}  // namespace avela
