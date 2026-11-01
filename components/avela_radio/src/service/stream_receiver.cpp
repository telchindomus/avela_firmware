#include "avela_radio/service/stream_receiver.hpp"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <iterator>
#include <memory>

#include "avela_hal/memory/heap_watch.hpp"
#include "esp_app_desc.h"
#include "esp_crt_bundle.h"
#include "esp_log.h"

namespace avela {
namespace {

constexpr const char* log_tag = "stream";
constexpr int http_timeout_ms = 5000;
constexpr int max_read_timeouts = 3;  // in a row (15 s without a byte): the station stalled, connect again
constexpr int http_ok = 200;
constexpr int max_redirects = 5;
constexpr TickType_t retry_after_failure = pdMS_TO_TICKS(5000);
constexpr TickType_t queue_wait = pdMS_TO_TICKS(100);  // how often a full queue checks for a new request
constexpr UBaseType_t priority = 4;                    // below the network stack, above the main loop
constexpr BaseType_t core = 0;                         // with the network stack; the player has the other

bool is_redirect(int status)
{
    constexpr int moved_permanently = 301;
    constexpr int found = 302;
    constexpr int see_other = 303;
    constexpr int temporary_redirect = 307;
    constexpr int permanent_redirect = 308;
    return status == moved_permanently || status == found || status == see_other || status == temporary_redirect ||
           status == permanent_redirect;
}

}  // namespace

//--------------------------------------------------------------------------------------------------

void StreamReceiver::init()
{
    m_user_agent_.append("Avela/").append(std::data(esp_app_get_description()->version));
    m_tune_requests_ =
        xQueueCreateStatic(1, sizeof(TuneRequest), m_tune_requests_items_.data(), &m_tune_requests_storage_);
    m_encoded_audio_ = xQueueCreateStatic(chunk_count, sizeof(EncodedAudioChunk), m_encoded_audio_items_.data(),
                                          &m_encoded_audio_storage_);
    m_latest_reception_ =
        xQueueCreateStatic(1, sizeof(ReceptionReport), m_latest_reception_items_.data(), &m_latest_reception_storage_);
    m_task_handle_ = xTaskCreateStaticPinnedToCore(&StreamReceiver::run, task_name, m_stack_.size(), this, priority,
                                                   m_stack_.data(), &m_task_, core);
}

//--------------------------------------------------------------------------------------------------

uint32_t StreamReceiver::stack_left() const
{
    return uxTaskGetStackHighWaterMark(m_task_handle_);
}

//--------------------------------------------------------------------------------------------------

// The request queue holds one: a newer request replaces one the task hasn't taken yet.
void StreamReceiver::tune(std::string_view url)
{
    TuneRequest request;
    request.m_url.append(url);
    xQueueOverwrite(m_tune_requests_, &request);
}

//--------------------------------------------------------------------------------------------------

bool StreamReceiver::take_report(ReceptionReport& report)
{
    return xQueueReceive(m_latest_reception_, &report, 0) == pdTRUE;
}

//--------------------------------------------------------------------------------------------------

void StreamReceiver::run(void* self)
{
    static_cast<StreamReceiver*>(self)->run();
}

//--------------------------------------------------------------------------------------------------

// Every pass is a new session: a new request, or the same station again after a failure or the end of its stream.
void StreamReceiver::run()
{
    TuneRequest request;
    while (true) {
        m_session_.fetch_add(1);
        m_report_.m_title.clear();
        if (request.m_url.empty()) {
            publish(ReceptionStatus::idle);
            xQueueReceive(m_tune_requests_, &request, portMAX_DELAY);
            continue;
        }
        publish(ReceptionStatus::connecting);
        switch (receive(request.m_url.c_str())) {
        case Outcome::new_request:
            xQueueReceive(m_tune_requests_, &request, 0);
            break;
        case Outcome::ended:
            ESP_LOGI(log_tag, "the station ended the stream; connecting again");
            break;
        case Outcome::failed:
            publish(ReceptionStatus::failed);
            xQueueReceive(m_tune_requests_, &request, retry_after_failure);  // else the same station again
            break;
        }
    }
}

//--------------------------------------------------------------------------------------------------

StreamReceiver::Outcome StreamReceiver::receive(const char* url)
{
    const HeapWatch heap_watch;  // what the connection costs: logged once it is open
    esp_http_client_config_t config{};
    config.url = url;
    config.user_agent = m_user_agent_.c_str();
    config.timeout_ms = http_timeout_ms;
    config.crt_bundle_attach = esp_crt_bundle_attach;
    config.event_handler = &StreamReceiver::on_http_event;
    config.user_data = this;
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == nullptr) {
        return Outcome::failed;
    }
    esp_http_client_set_header(client, "Icy-MetaData", "1");

    Outcome outcome = Outcome::failed;
    if (connect(client)) {
        ESP_LOGI(log_tag, "receiving %s (metadata every %zu bytes)", url, m_audio_interval_);
        heap_watch.log(log_tag, "connected");
        outcome = stream(client);
    }
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    return outcome;
}

//--------------------------------------------------------------------------------------------------

StreamReceiver::Outcome StreamReceiver::stream(esp_http_client_handle_t client)
{
    IcyMetadataReader metadata{m_audio_interval_};
    m_chunk_.m_session = m_session_.load();
    m_chunk_.m_size = 0;
    int read_timeouts = 0;
    while (true) {
        if (request_waiting()) {
            return Outcome::new_request;
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast): the HTTP client's C API reads into char
        const int read = esp_http_client_read(client, reinterpret_cast<char*>(m_received_.data()),
                                              static_cast<int>(m_received_.size()));
        if (read == -ESP_ERR_HTTP_EAGAIN) {
            if (++read_timeouts < max_read_timeouts) {
                continue;  // nothing for a while; the connection may still come back
            }
            ESP_LOGW(log_tag, "no data for %d s: connecting again", max_read_timeouts * http_timeout_ms / 1000);
            return Outcome::failed;  // retried like any failed connection
        }
        read_timeouts = 0;
        if (read <= 0) {
            return read == 0 ? Outcome::ended : Outcome::failed;
        }
        const uint32_t title_revision = metadata.title_revision();
        const std::span<uint8_t> audio = metadata.strip(std::span{m_received_}.first(static_cast<std::size_t>(read)));
        if (metadata.title_revision() != title_revision) {
            m_report_.m_title.clear().append(metadata.title());
            publish(m_report_.m_status);
        }
        if (!queue_audio(audio)) {
            return Outcome::new_request;
        }
    }
}

//--------------------------------------------------------------------------------------------------

bool StreamReceiver::connect(esp_http_client_handle_t client)
{
    for (int redirects = 0;; ++redirects) {
        m_audio_interval_ = 0;
        m_audio_answer_ = false;
        if (esp_http_client_open(client, 0) != ESP_OK || esp_http_client_fetch_headers(client) < 0) {
            ESP_LOGW(log_tag, "no answer");
            return false;
        }
        const int status = esp_http_client_get_status_code(client);
        if (is_redirect(status) && redirects < max_redirects && esp_http_client_set_redirection(client) == ESP_OK) {
            esp_http_client_close(client);
            continue;
        }
        if (status != http_ok || !m_audio_answer_) {
            ESP_LOGW(log_tag, "answer %d%s", status, m_audio_answer_ ? "" : ", not an MP3 stream");
            return false;
        }
        return true;
    }
}

//--------------------------------------------------------------------------------------------------

bool StreamReceiver::queue_audio(std::span<const uint8_t> audio)
{
    while (!audio.empty()) {
        const std::size_t count = std::min(audio.size(), m_chunk_.m_bytes.size() - m_chunk_.m_size);
        std::copy_n(audio.begin(), count, m_chunk_.m_bytes.begin() + static_cast<std::ptrdiff_t>(m_chunk_.m_size));
        m_chunk_.m_size += count;
        audio = audio.subspan(count);
        if (m_chunk_.m_size == m_chunk_.m_bytes.size()) {
            if (!queue_chunk()) {
                return false;
            }
            m_chunk_.m_size = 0;
        }
    }
    return true;
}

//--------------------------------------------------------------------------------------------------

// The queue is full most of the time: the station sends faster than the player plays, and the player's pace (the
// sample rate) sets the stream's.
bool StreamReceiver::queue_chunk()
{
    while (xQueueSend(m_encoded_audio_, &m_chunk_, queue_wait) != pdTRUE) {
        if (request_waiting()) {
            return false;
        }
    }
    if (m_report_.m_status != ReceptionStatus::receiving) {
        publish(ReceptionStatus::receiving);
    }
    return true;
}

//--------------------------------------------------------------------------------------------------

bool StreamReceiver::request_waiting() const
{
    return uxQueueMessagesWaiting(m_tune_requests_) > 0;
}

//--------------------------------------------------------------------------------------------------

void StreamReceiver::publish(ReceptionStatus status)
{
    m_report_.m_session = m_session_.load();
    m_report_.m_status = status;
    xQueueOverwrite(m_latest_reception_, &m_report_);
}

//--------------------------------------------------------------------------------------------------

// The answer's headers, as they come (on the task, inside esp_http_client_fetch_headers()).
esp_err_t StreamReceiver::on_http_event(esp_http_client_event_t* event)
{
    if (event->event_id != HTTP_EVENT_ON_HEADER) {
        return ESP_OK;
    }
    auto& receiver = *static_cast<StreamReceiver*>(event->user_data);
    const std::string_view key = event->header_key;
    const std::string_view value = event->header_value;
    const auto same_key = [&](std::string_view name) {
        return std::ranges::equal(key, name, [](char left, char right) {
            return std::tolower(static_cast<unsigned char>(left)) == std::tolower(static_cast<unsigned char>(right));
        });
    };
    if (same_key("icy-metaint")) {
        std::size_t interval = 0;
        if (std::from_chars(std::to_address(value.begin()), std::to_address(value.end()), interval).ec == std::errc{}) {
            receiver.m_audio_interval_ = interval;
        }
    } else if (same_key("content-type")) {
        receiver.m_audio_answer_ = value.starts_with("audio/mpeg");
    }
    return ESP_OK;
}

}  // namespace avela
