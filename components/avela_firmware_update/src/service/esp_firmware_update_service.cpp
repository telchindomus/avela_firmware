#include "avela_firmware_update/service/esp_firmware_update_service.hpp"

#include <algorithm>
#include <cstring>
#include <optional>
#include <span>

#include "avela_setup/html_writer.hpp"
#include "avela_setup/setup_server.hpp"
#include "esp_app_desc.h"
#include "esp_app_format.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_system.h"
#include "esp_timer.h"

namespace avela {
namespace {

constexpr const char* log_tag = "update";
constexpr int64_t microseconds_per_millisecond = 1000;
constexpr int64_t milliseconds_per_second = 1000;

// Where an image keeps its app description: after the image header and the first segment's header.
constexpr std::size_t app_description_offset = sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t);
// what is_avela_firmware() reads: the image's start up to the end of its app description
constexpr std::size_t app_header_bytes = app_description_offset + sizeof(esp_app_desc_t);

// The upload page: picks a file and sends it as the body of POST /update, showing the radio's answer.
constexpr std::string_view upload_page_body = R"(<section><h2>Firmware update</h2>
<p>On the radio, open Info and double-click the knob first; the radio shows the progress.</p>
<label for="file">Firmware file (avela_firmware.bin)</label><input id="file" type="file" accept=".bin">
<button id="send" type="button">Update</button><p id="result"></p>
<script>
document.getElementById('send').onclick = async () => {
  const file = document.getElementById('file').files[0], result = document.getElementById('result');
  if (!file) { result.textContent = 'Choose a file first.'; return; }
  result.textContent = 'Sending…';
  try {
    const answer = await fetch('/update', {method: 'POST', body: file});
    result.textContent = await answer.text();
  } catch (error) { result.textContent = 'The radio did not answer: ' + error; }
};
</script><a href="/">Back to setup</a></section>)";

int64_t now_ms()
{
    return esp_timer_get_time() / microseconds_per_millisecond;
}

esp_err_t answer(httpd_req_t* request, const char* status, std::string_view text)
{
    httpd_resp_set_status(request, status);
    httpd_resp_set_type(request, "text/plain; charset=utf-8");
    return httpd_resp_send(request, text.data(), static_cast<ssize_t>(text.size()));
}

}  // namespace

//--------------------------------------------------------------------------------------------------

void EspFirmwareUpdateService::init(httpd_handle_t http_server)
{
    m_lock_ = xSemaphoreCreateMutexStatic(&m_lock_storage_);

    const esp_partition_t* running = esp_ota_get_running_partition();
    esp_ota_img_states_t state = ESP_OTA_IMG_UNDEFINED;
    m_confirmation_pending_ =
        esp_ota_get_state_partition(running, &state) == ESP_OK && state == ESP_OTA_IMG_PENDING_VERIFY;
    if (m_confirmation_pending_) {
        ESP_LOGI(log_tag, "new firmware in %s: confirmed after %lld s without a restart", std::data(running->label),
                 confirm_after_ms / milliseconds_per_second);
    }

    if (http_server == nullptr) {
        return;
    }
    const httpd_uri_t page_uri{.uri = "/update", .method = HTTP_GET, .handler = &page, .user_ctx = this};
    const httpd_uri_t upload_uri{.uri = "/update", .method = HTTP_POST, .handler = &upload, .user_ctx = this};
    ESP_ERROR_CHECK(httpd_register_uri_handler(http_server, &page_uri));
    ESP_ERROR_CHECK(httpd_register_uri_handler(http_server, &upload_uri));
}

//--------------------------------------------------------------------------------------------------

void EspFirmwareUpdateService::update(FirmwareUpdateModel& model)
{
    while (const std::optional<AcceptUploadsRequest> request = m_requests_.take()) {
        if (request->m_accept && !m_accepting_.load()) {
            // the screen just opened: a fresh start, not the last attempt's failure
            m_state_.store(FirmwareUpdateState::waiting);
        }
        m_accepting_.store(request->m_accept);
    }
    const int64_t now = now_ms();
    model.m_running_version = std::data(esp_app_get_description()->version);
    model.m_state = m_state_.load();
    model.m_received_bytes = m_received_bytes_.load();
    model.m_total_bytes = m_total_bytes_.load();
    if (model.m_state == FirmwareUpdateState::failed) {
        xSemaphoreTake(m_lock_, portMAX_DELAY);
        model.m_failure.clear();
        model.m_failure.append(m_failure_.view());
        xSemaphoreGive(m_lock_);
    } else {
        model.m_failure.clear();
    }

    confirm_running_firmware(now);
    if (model.m_state == FirmwareUpdateState::restarting) {
        if (m_restart_at_ms_ == 0) {
            m_restart_at_ms_ = now + restart_delay_ms;
        } else if (now >= m_restart_at_ms_) {
            ESP_LOGI(log_tag, "restarting into the new firmware");
            esp_restart();
        }
    }
}

//--------------------------------------------------------------------------------------------------

void EspFirmwareUpdateService::confirm_running_firmware(int64_t now)
{
    if (!m_confirmation_pending_ || now < confirm_after_ms) {
        return;
    }
    m_confirmation_pending_ = false;
    if (const esp_err_t result = esp_ota_mark_app_valid_cancel_rollback(); result != ESP_OK) {
        ESP_LOGE(log_tag, "firmware not confirmed: %s", esp_err_to_name(result));
        return;
    }
    ESP_LOGI(log_tag, "firmware confirmed: the bootloader keeps it");
}

//--------------------------------------------------------------------------------------------------

esp_err_t EspFirmwareUpdateService::page(httpd_req_t* request)
{
    if (!setup_page::allowed(request)) {
        return setup_page::after_refusal(request);
    }
    httpd_resp_set_type(request, "text/html; charset=utf-8");
    HtmlWriter writer{request};
    setup_page::write_head(writer);
    writer.raw(upload_page_body);
    setup_page::write_tail(writer);
    return writer.finish() ? ESP_OK : ESP_FAIL;
}

//--------------------------------------------------------------------------------------------------

esp_err_t EspFirmwareUpdateService::upload(httpd_req_t* request)
{
    if (!setup_page::allowed(request)) {
        return setup_page::after_refusal(request);
    }
    auto& service = *static_cast<EspFirmwareUpdateService*>(request->user_ctx);
    if (!service.m_accepting_.load()) {
        return answer(request, "403 Forbidden",
                      "The radio doesn't accept updates now: open Info and double-click the knob first.");
    }
    if (service.m_state_.load() == FirmwareUpdateState::receiving ||
        service.m_state_.load() == FirmwareUpdateState::restarting) {
        return answer(request, "409 Conflict", "An update is already running.");
    }
    if (!service.receive(request)) {
        xSemaphoreTake(service.m_lock_, portMAX_DELAY);
        const FixedString<FirmwareUpdateModel::failure_capacity> failure = service.m_failure_;
        xSemaphoreGive(service.m_lock_);
        return answer(request, "400 Bad Request", failure.view());
    }
    return answer(request, "200 OK", "Updated. The radio restarts into the new firmware.");
}

//--------------------------------------------------------------------------------------------------

bool EspFirmwareUpdateService::receive(httpd_req_t* request)
{
    static_assert(chunk_bytes >= app_header_bytes, "the first chunk holds the app description");
    const esp_partition_t* slot = esp_ota_get_next_update_partition(nullptr);
    if (slot == nullptr) {
        fail("This radio's flash has no second app slot.");
        return false;
    }
    if (request->content_len > slot->size) {
        fail("The file is larger than an app slot.");
        return false;
    }
    if (request->content_len < app_header_bytes) {
        fail("The file is too short to be a firmware.");
        return false;
    }
    m_received_bytes_.store(0);
    m_total_bytes_.store(request->content_len);
    m_state_.store(FirmwareUpdateState::receiving);
    ESP_LOGI(log_tag, "receiving %zu bytes into %s", request->content_len, std::data(slot->label));

    esp_ota_handle_t handle = 0;
    if (const esp_err_t result = esp_ota_begin(slot, OTA_WITH_SEQUENTIAL_WRITES, &handle); result != ESP_OK) {
        fail("The app slot couldn't be prepared.");
        return false;
    }
    // A receive returns what has come so far, so the first chunk is collected until it holds the app description
    // (the file is at least that long) before it is checked; later chunks are written as they come.
    std::size_t received = 0;  // written to the slot
    std::size_t buffered = 0;  // in m_chunk_, not written yet
    bool checked = false;
    while (received + buffered < request->content_len) {
        const std::span<char> free = std::span{m_chunk_}.subspan(
            buffered, std::min(m_chunk_.size() - buffered, request->content_len - received - buffered));
        const int count = setup_page::receive(request, free);
        if (count <= 0) {
            esp_ota_abort(handle);
            fail("The upload broke off.");
            return false;
        }
        buffered += static_cast<std::size_t>(count);
        if (!checked) {
            if (buffered < app_header_bytes) {
                continue;
            }
            if (!is_avela_firmware({m_chunk_.data(), buffered})) {
                esp_ota_abort(handle);
                return false;
            }
            checked = true;
        }
        if (esp_ota_write(handle, m_chunk_.data(), buffered) != ESP_OK) {
            esp_ota_abort(handle);
            fail("Writing the flash failed.");
            return false;
        }
        received += buffered;
        buffered = 0;
        m_received_bytes_.store(received);
    }
    if (const esp_err_t result = esp_ota_end(handle); result != ESP_OK) {
        fail(result == ESP_ERR_OTA_VALIDATE_FAILED ? "The image is damaged or not for this chip."
                                                   : "The image couldn't be finished.");
        return false;
    }
    if (esp_ota_set_boot_partition(slot) != ESP_OK) {
        fail("The new firmware couldn't be chosen for the next start.");
        return false;
    }
    ESP_LOGI(log_tag, "new firmware in %s, restarting soon", std::data(slot->label));
    m_state_.store(FirmwareUpdateState::restarting);
    return true;
}

//--------------------------------------------------------------------------------------------------

bool EspFirmwareUpdateService::is_avela_firmware(std::string_view first_chunk)
{
    if (first_chunk.size() < app_header_bytes) {
        fail("The file is too short to be a firmware.");
        return false;
    }
    esp_app_desc_t uploaded{};
    std::memcpy(&uploaded, first_chunk.substr(app_description_offset).data(), sizeof(uploaded));
    const esp_app_desc_t* running = esp_app_get_description();
    // project names fill their array, NUL-terminated if shorter
    const auto project_name = [](const esp_app_desc_t& description) {
        return std::string_view{std::data(description.project_name),
                                strnlen(std::data(description.project_name), std::size(description.project_name))};
    };
    if (uploaded.magic_word != ESP_APP_DESC_MAGIC_WORD || project_name(uploaded) != project_name(*running)) {
        fail("This isn't an Avela firmware.");
        return false;
    }
    return true;
}

//--------------------------------------------------------------------------------------------------

void EspFirmwareUpdateService::fail(std::string_view reason)
{
    ESP_LOGW(log_tag, "update failed: %.*s", static_cast<int>(reason.size()), reason.data());
    xSemaphoreTake(m_lock_, portMAX_DELAY);
    m_failure_.clear();
    m_failure_.append(reason);
    xSemaphoreGive(m_lock_);
    m_state_.store(FirmwareUpdateState::failed);
}

}  // namespace avela
