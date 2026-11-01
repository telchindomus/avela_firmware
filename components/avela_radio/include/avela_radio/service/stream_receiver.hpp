// The stream task: connects to the station it is asked for (HTTP or HTTPS, following redirects), takes the stream
// titles out (IcyMetadataReader) and queues the MP3 bytes for the player in chunks. Each connection is a session of its
// own: its chunks carry its number, and a chunk of an older session is stale (a station no longer wanted, or a
// connection that broke off). When a connection fails it tries again a few seconds later; when the station ends the
// stream, at once. ESP-IDF's HTTP client allocates its buffers for each connection (a library's heap use).
//
// Between the tasks, only queues (no shared memory, nothing to lock):
// - m_tune_requests_: main loop -> this task, a latest-value queue (one item, a newer one overwrites it): what to
//   play (TuneRequest);
// - m_encoded_audio_: this task -> the player, the MP3 bytes (EncodedAudioChunk);
// - m_latest_reception_: this task -> main loop, a latest-value queue: how the connection goes and the stream's
//   title (ReceptionReport).
// And one number, m_session_ (atomic): the session it is in, for the player to skip stale chunks.
#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <string_view>

#include "avela_radio/model/radio_model.hpp"
#include "avela_radio/service/encoded_audio_chunk.hpp"
#include "avela_radio/service/reception_report.hpp"
#include "avela_radio/service/reception_status.hpp"
#include "avela_radio/service/tune_request.hpp"
#include "avela_radio/stream/icy_metadata_reader.hpp"
#include "avela_strings/fixed_string.hpp"
#include "esp_app_desc.h"
#include "esp_http_client.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

namespace avela {

class StreamReceiver {
public:
    // 12 KB: about 0.75 s of a 128 kbit/s stream, to ride out the network's hiccups
    static constexpr std::size_t chunk_count = 12;
    static constexpr const char* task_name = "stream";
    static constexpr uint32_t stack_bytes = 8192;  // TLS handshakes (HTTPS stations) need a deep stack

    StreamReceiver() = default;
    StreamReceiver(const StreamReceiver&) = delete;
    StreamReceiver& operator=(const StreamReceiver&) = delete;
    StreamReceiver(StreamReceiver&&) = delete;
    StreamReceiver& operator=(StreamReceiver&&) = delete;
    ~StreamReceiver() = default;

    // Creates the queues and starts the task (stopped).
    void init();

    // Main loop: play this station, from a new connection (an empty url = stop).
    void tune(std::string_view url);
    // Main loop: the newest report, if one came since the last call.
    bool take_report(ReceptionReport& report);

    // For the player: the chunks, and the session they have to belong to.
    [[nodiscard]] QueueHandle_t encoded_audio() const { return m_encoded_audio_; }
    [[nodiscard]] uint32_t session() const { return m_session_.load(); }
    // For the diagnostics log: the fewest bytes the task's stack has had left.
    [[nodiscard]] uint32_t stack_left() const;

private:
    enum class Outcome : uint8_t {
        new_request,  // stopped receiving because a new request came
        ended,        // the station closed the stream
        failed,
    };

    static constexpr std::size_t read_size = 1024;

    static void run(void* self);
    [[noreturn]] void run();
    // Receives url into the queue until a new request comes, the stream ends or fails.
    Outcome receive(const char* url);
    // Connects (following redirects) and checks the answer is an MP3 stream; false if not.
    bool connect(esp_http_client_handle_t client);
    // Receives the connected stream into the queue until a new request comes, it ends, fails or stalls
    // (max_read_timeouts read timeouts in a row).
    Outcome stream(esp_http_client_handle_t client);
    // Copies audio into the chunk being filled and queues full chunks; false if a new request came meanwhile.
    bool queue_audio(std::span<const uint8_t> audio);
    // Queues the chunk being filled, waiting while the queue is full; false if a new request came meanwhile.
    bool queue_chunk();
    [[nodiscard]] bool request_waiting() const;
    void publish(ReceptionStatus status);
    static esp_err_t on_http_event(esp_http_client_event_t* event);

    std::atomic<uint32_t> m_session_{0};
    // "Avela/0.1.0": SomaFM and others turn away clients that don't name themselves.
    FixedString<capacity_for("Avela/") + sizeof(esp_app_desc_t::version)> m_user_agent_;

    // the task's state
    ReceptionReport m_report_;
    EncodedAudioChunk m_chunk_;         // being filled
    std::size_t m_audio_interval_ = 0;  // the answer's icy-metaint (0 = no metadata)
    bool m_audio_answer_ = false;       // the answer's Content-Type is audio/mpeg
    std::array<uint8_t, read_size> m_received_{};

    StaticQueue_t m_tune_requests_storage_{};
    std::array<uint8_t, sizeof(TuneRequest)> m_tune_requests_items_{};
    QueueHandle_t m_tune_requests_ = nullptr;
    StaticQueue_t m_encoded_audio_storage_{};
    std::array<uint8_t, sizeof(EncodedAudioChunk) * chunk_count> m_encoded_audio_items_{};
    QueueHandle_t m_encoded_audio_ = nullptr;
    StaticQueue_t m_latest_reception_storage_{};
    std::array<uint8_t, sizeof(ReceptionReport)> m_latest_reception_items_{};
    QueueHandle_t m_latest_reception_ = nullptr;

    StaticTask_t m_task_{};
    TaskHandle_t m_task_handle_ = nullptr;
    std::array<StackType_t, stack_bytes / sizeof(StackType_t)> m_stack_{};
};

}  // namespace avela
