// The weather from Open-Meteo (open-meteo.com, no API key): while the radio is on the home network, it looks up the
// place saved on the setup page once (geocoding API; the coordinates are stored in WeatherSettings) and then asks for
// the current weather and the next days' forecast every few minutes (menuconfig: Avela weather), at once when the
// settings change or the screen asks for it, and again a minute after a failure.
// HTTPS requests block for seconds, so they run on a task of their own (static stack); it hands a finished report over
// under a lock, and update() copies it into the model on the main loop's task. Atomics carry the other way: whether
// the home network is joined and whether an update was requested. ESP-IDF's HTTP client and TLS allocate their buffers
// for each request (the only heap use, a library's, and given back afterwards).
#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <string_view>

#include "avela_messaging/messages/weather_update_request.hpp"
#include "avela_messaging/queues/request_queue.hpp"
#include "avela_network/model/network_model.hpp"
#include "avela_weather/model/weather_model.hpp"
#include "avela_weather/model/weather_settings.hpp"
#include "avela_weather/service/weather_service_concept.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

namespace avela {

class OpenMeteoWeatherService {
public:
    // network: read in update() to know whether the home network is joined
    explicit OpenMeteoWeatherService(const NetworkModel& network);
    OpenMeteoWeatherService(const OpenMeteoWeatherService&) = delete;
    OpenMeteoWeatherService& operator=(const OpenMeteoWeatherService&) = delete;
    OpenMeteoWeatherService(OpenMeteoWeatherService&&) = delete;
    OpenMeteoWeatherService& operator=(OpenMeteoWeatherService&&) = delete;
    ~OpenMeteoWeatherService() = default;

    void init();
    void update(WeatherModel& model);
    // Where its clients ask for an update now (handed to the task in update()).
    RequestQueue<WeatherUpdateRequest>& requests() { return m_requests_; }

    static constexpr const char* task_name = "weather";
    static constexpr uint32_t stack_bytes = 8192;  // TLS handshakes need a deep stack

private:
    static constexpr std::size_t response_capacity = 4096;

    static void run(void* self);
    void run();
    // Takes the saved settings when they changed (the setup page saved), and then updates at once; true if that
    // emptied the report (another place or other units), which then has to be published.
    bool follow_settings();
    // ready: an update may run; else why not (no place, not on the home network).
    [[nodiscard]] WeatherStatus waiting_status() const;
    // Whether it is time to update (the interval passed, the screen asked, or the network was just joined).
    bool update_due();
    // Looks the place up if needed, fetches, publishes and sets the next update's time.
    void update_now();
    // The settings saved last; false if there are none.
    static bool load_settings(WeatherSettings& settings);
    // Looks the place up and puts its coordinates into `settings` (not stored); false if that didn't work (the report
    // says why).
    bool locate(WeatherSettings& settings);
    // Stores the coordinates `looked_up` found for m_settings_'s place; true if the stored settings changed (with the
    // coordinates, or saved again during the lookup), so follow_settings() takes them, report and all. False if NVS
    // refused: m_settings_ then takes the coordinates until a restart.
    bool store_location(const WeatherSettings& looked_up);
    // Asks for the weather at the stored coordinates; false if that didn't work.
    bool fetch(const WeatherSettings& settings);
    // GET over HTTPS into m_response_; the length of the body, or nothing if the request failed.
    bool get(const char* url, std::string_view& body);
    // Hands the task's report to the main loop (with a new revision).
    void publish();

    const NetworkModel& m_network_;
    std::atomic<bool> m_joined_{false};  // main loop -> task
    RequestQueue<WeatherUpdateRequest> m_requests_;
    std::atomic<bool> m_update_requested_{false};  // main loop -> task

    // the task's state
    WeatherSettings m_settings_{};
    bool m_has_settings_ = false;
    bool m_settings_read_ = false;
    uint32_t m_settings_revision_ = 0;
    int64_t m_next_update_ms_ = 0;

    // the task's own working copy, and the one handed over (under m_lock_)
    WeatherReport m_report_;
    WeatherReport m_published_;
    std::atomic<uint32_t> m_published_revision_{0};
    uint32_t m_copied_revision_ = 0;  // main loop: the last one copied into the model
    StaticSemaphore_t m_lock_storage_{};
    SemaphoreHandle_t m_lock_ = nullptr;

    std::array<char, response_capacity> m_response_{};
    StaticTask_t m_task_{};
    std::array<StackType_t, stack_bytes / sizeof(StackType_t)> m_stack_{};
};

static_assert(WeatherServiceConcept<OpenMeteoWeatherService>);

}  // namespace avela
