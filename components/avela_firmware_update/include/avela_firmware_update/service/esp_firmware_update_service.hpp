// Firmware updates with ESP-IDF's OTA: the flash has two app slots (partitions.csv); a new firmware is written into
// the one that doesn't run, checked, and booted on the next start. Uploads arrive on the setup page's HTTP server:
// GET /update is a small page that sends the chosen file, POST /update takes the image as the request body (the page,
// or `avela-tuner update`). Both need the page password on the home network (setup_page::allowed). An upload is
// accepted only while the update screen is open (FirmwareUpdateModel), and only if it is an Avela firmware for this
// chip.
// The bootloader's rollback (CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE): a new firmware starts "pending"; once it has run
// for a while this service confirms it. If it crashes or restarts before that, the bootloader goes back to the one that
// ran before.
// The upload is handled on the HTTP server's task; it hands its progress to update() (main loop) through atomics and
// the failure text under a lock. update() restarts the radio once an image is in place.
#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <string_view>

#include "avela_firmware_update/model/firmware_update_model.hpp"
#include "avela_messaging/messages/accept_uploads_request.hpp"
#include "avela_messaging/queues/request_queue.hpp"
#include "esp_http_server.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace avela {

class EspFirmwareUpdateService {
public:
    EspFirmwareUpdateService() = default;
    EspFirmwareUpdateService(const EspFirmwareUpdateService&) = delete;
    EspFirmwareUpdateService& operator=(const EspFirmwareUpdateService&) = delete;
    EspFirmwareUpdateService(EspFirmwareUpdateService&&) = delete;
    EspFirmwareUpdateService& operator=(EspFirmwareUpdateService&&) = delete;
    ~EspFirmwareUpdateService() = default;

    // Once, after the setup page's server started: notes whether the running firmware waits for its confirmation and
    // adds the update page to the server (nothing if it didn't start).
    void init(httpd_handle_t http_server);
    // Every frame: passes the screen's request on, reports the upload, confirms the running firmware once it has run
    // long enough, and restarts into a new one.
    void update(FirmwareUpdateModel& model);
    // Where its clients ask it to accept uploads, or refuse them again (carried out in update()).
    RequestQueue<AcceptUploadsRequest>& requests() { return m_requests_; }

private:
    // what the network hands over at a time; esp_ota_write erases the flash sectors as the image reaches them
    static constexpr std::size_t chunk_bytes = 1024;
    static constexpr int64_t confirm_after_ms = 30'000;
    static constexpr int64_t restart_delay_ms = 1500;  // the answer goes out and the screen shows "restarting"

    static esp_err_t page(httpd_req_t* request);
    static esp_err_t upload(httpd_req_t* request);
    // Takes the request's body into the free app slot; false (the failure set) if it was refused or broke off.
    bool receive(httpd_req_t* request);
    // Whether the image's app description (in its first chunk) is an Avela firmware; the failure set if not.
    bool is_avela_firmware(std::string_view first_chunk);
    void fail(std::string_view reason);
    void confirm_running_firmware(int64_t now_ms);

    // main loop -> upload
    RequestQueue<AcceptUploadsRequest> m_requests_;
    std::atomic<bool> m_accepting_{false};
    // upload -> main loop
    std::atomic<FirmwareUpdateState> m_state_{FirmwareUpdateState::waiting};
    std::atomic<std::size_t> m_received_bytes_{0};
    std::atomic<std::size_t> m_total_bytes_{0};
    FixedString<FirmwareUpdateModel::failure_capacity> m_failure_;  // under m_lock_
    StaticSemaphore_t m_lock_storage_{};
    SemaphoreHandle_t m_lock_ = nullptr;

    // main loop only
    bool m_confirmation_pending_ = false;
    int64_t m_restart_at_ms_ = 0;  // 0: no restart due

    // the HTTP server's task only (one task handles every request)
    std::array<char, chunk_bytes> m_chunk_{};
};

}  // namespace avela
