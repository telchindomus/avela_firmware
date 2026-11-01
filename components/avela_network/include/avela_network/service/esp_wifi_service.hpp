// Wi-Fi with ESP-IDF. Keeps the radio on the saved home network: joins it and retries every retry_delay_ms, forever
// (a router that boots slower than the radio after a power cut is joined by itself). No network saved: nothing to
// do (not_configured). The radio's own setup network opens only when the model requests it (Info screen) and
// closes when the request is withdrawn or a network is saved on the setup page, which is then joined right away.
// While the setup network is up the radio makes no join attempts (an attempt scans other channels and would drop the
// phone on the setup network), and the captive DNS sends phones to the page. Once joined for the first time: SNTP sets
// the SystemClock, mDNS announces http://<host name>.local. ESP-IDF reports events on its event task: the handler only
// stores them in atomics; update() (main loop) acts.
#pragma once

#include <array>
#include <atomic>
#include <cstdint>
#include <optional>

#include "avela_messaging/messages/setup_network_request.hpp"
#include "avela_messaging/queues/request_queue.hpp"
#include "avela_network/model/host_name.hpp"
#include "avela_network/model/network_model.hpp"
#include "avela_network/model/wifi_credentials.hpp"
#include "avela_network/service/captive_dns.hpp"
#include "avela_network/service/network_service_concept.hpp"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi_types.h"

namespace avela {

class EspWifiService {
public:
    void init();
    void update(NetworkModel& model);
    // Where its clients ask it to open or close the setup network (carried out in update()).
    RequestQueue<SetupNetworkRequest>& requests() { return m_requests_; }

private:
    static constexpr uint32_t retry_delay_ms = 30000;      // after a failed attempt
    static constexpr uint32_t attempt_timeout_ms = 20000;  // an attempt that never reports back
    static constexpr uint32_t signal_interval_ms = 2000;

    // What the station interface is busy with.
    enum class Activity : uint8_t { idle, joining };

    void follow_settings(NetworkModel& model, uint32_t now_ms);
    void follow_setup_network(NetworkModel& model);
    void follow_events(NetworkModel& model, uint32_t now_ms);
    void start_next_activity(const NetworkModel& model, uint32_t now_ms);
    void open_setup_network(NetworkModel& model);
    void close_setup_network(NetworkModel& model);
    void report_signal(NetworkModel& model, uint32_t now_ms);
    void start_internet_services();

    static void on_event(void* self, esp_event_base_t base, int32_t id, void* data);  // event task

    // written by the event task
    std::atomic<uint32_t> m_disconnects_{0};
    std::atomic<uint32_t> m_address_{0};  // IPv4 on the home network, network byte order; 0 = none

    // main loop only
    esp_netif_t* m_station_ = nullptr;
    esp_netif_t* m_access_point_ = nullptr;
    HostName m_host_name_{};  // in force since init()
    WifiCredentials m_credentials_{};
    bool m_has_credentials_ = false;
    RequestQueue<SetupNetworkRequest> m_requests_;
    bool m_setup_network_wanted_ = false;  // the last SetupNetworkRequest; cleared when a network is saved
    std::optional<uint32_t> m_credentials_revision_;
    Activity m_activity_ = Activity::idle;
    uint32_t m_activity_since_ms_ = 0;
    uint32_t m_seen_disconnects_ = 0;
    uint32_t m_next_attempt_ms_ = 0;
    uint32_t m_next_signal_ms_ = 0;
    bool m_internet_services_started_ = false;
    CaptiveDns m_dns_;
};

static_assert(NetworkServiceConcept<EspWifiService>);

}  // namespace avela
