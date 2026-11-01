#include "avela_radio/service/mp3_player.hpp"

#include "avela_radio/audio/volume.hpp"
#include "esp_log.h"

namespace avela {
namespace {

constexpr const char* log_tag = "player";
constexpr UBaseType_t priority = 5;  // above the stream task: the sound must not stop while the network is busy
constexpr BaseType_t core = 1;       // away from the network stack and the main loop
constexpr std::size_t start_after_chunks = StreamReceiver::chunk_count / 2;
constexpr TickType_t fill_wait = pdMS_TO_TICKS(20);
constexpr TickType_t fill_timeout = pdMS_TO_TICKS(3000);  // a slow station starts anyway

}  // namespace

//--------------------------------------------------------------------------------------------------

void Mp3Player::init()
{
    m_decoder_.init();
    m_analyzer_.init();
    m_latest_playback_ =
        xQueueCreateStatic(1, sizeof(PlaybackReport), m_latest_playback_items_.data(), &m_latest_playback_storage_);
    m_task_handle_ = xTaskCreateStaticPinnedToCore(&Mp3Player::run, task_name, m_stack_.size(), this, priority,
                                                   m_stack_.data(), &m_task_, core);
}

//--------------------------------------------------------------------------------------------------

void Mp3Player::set_volume(int volume)
{
    m_volume_gain_.store(volume_gain(volume));
}

//--------------------------------------------------------------------------------------------------

bool Mp3Player::take_report(PlaybackReport& report)
{
    return xQueueReceive(m_latest_playback_, &report, 0) == pdTRUE;
}

//--------------------------------------------------------------------------------------------------

uint32_t Mp3Player::stack_left() const
{
    return uxTaskGetStackHighWaterMark(m_task_handle_);
}

//--------------------------------------------------------------------------------------------------

void Mp3Player::run(void* self)
{
    static_cast<Mp3Player*>(self)->run();
}

//--------------------------------------------------------------------------------------------------

void Mp3Player::run()
{
    while (true) {
        if (xQueueReceive(m_receiver_.encoded_audio(), &m_chunk_, silence_after) != pdTRUE) {
            if (m_report_.m_sounding) {
                if (m_receiver_.session() == m_report_.m_session) {
                    m_dropouts_.fetch_add(1);  // the station is still wanted: the stream fell behind
                }
                m_report_.m_sounding = false;
                m_report_.m_spectrum.fill(0);
                publish();
            }
            // the last frame has long played: silence_after is longer than the DMA buffers. Also when this session
            // never sounded: the one before may have left the output running (stop() does nothing if it's stopped)
            m_output_.stop();
            continue;
        }
        if (m_chunk_.m_session != m_receiver_.session()) {
            continue;  // stale: a station no longer wanted
        }
        if (m_chunk_.m_session != m_report_.m_session) {
            start_session(m_chunk_.m_session);
            if (m_chunk_.m_session != m_receiver_.session()) {
                continue;  // stopped or retuned while the buffer filled
            }
        }
        play_chunk();
    }
}

//--------------------------------------------------------------------------------------------------

void Mp3Player::start_session(uint32_t session)
{
    m_decoder_.reset();
    m_report_ = PlaybackReport{};
    m_report_.m_session = session;
    m_kilobits_per_second_ = 0;
    publish();
    const TickType_t started = xTaskGetTickCount();
    while (uxQueueMessagesWaiting(m_receiver_.encoded_audio()) < start_after_chunks &&
           m_receiver_.session() == session && xTaskGetTickCount() - started < fill_timeout) {
        vTaskDelay(fill_wait);
    }
}

//--------------------------------------------------------------------------------------------------

void Mp3Player::play_chunk()
{
    std::span<uint8_t> input = std::span{m_chunk_.m_bytes}.first(m_chunk_.m_size);
    while (!input.empty()) {
        const Mp3Decoder::Result result = m_decoder_.decode(input);
        if (result.m_failed) {
            ESP_LOGW(log_tag, "not MP3, or damaged: skipped %zu bytes", input.size());
            return;
        }
        if (result.m_frame) {
            play_frame(*result.m_frame);
        }
        if (result.m_consumed == 0) {
            // the simple decoder keeps partial frames itself and takes every byte; if not, the rest is dropped
            // (the decoder finds the next frame)
            return;
        }
        input = input.subspan(result.m_consumed);
    }
}

//--------------------------------------------------------------------------------------------------

void Mp3Player::play_frame(const Mp3Decoder::Frame& frame)
{
    // the spectrum shows the station, not the knob: measured before the volume
    m_analyzer_.analyze(frame.m_samples, frame.m_sample_rate, m_report_.m_spectrum);
    apply_gain(frame.m_samples, m_volume_gain_.load());
    if (frame.m_kilobits_per_second != m_kilobits_per_second_) {
        m_kilobits_per_second_ = frame.m_kilobits_per_second;
        m_report_.m_format.clear();
        if (m_kilobits_per_second_ > 0) {  // the decoder knows it from the frame header
            m_report_.m_format.append_number(m_kilobits_per_second_).append(" kbit/s ");
        }
        m_report_.m_format.append("MP3");
    }
    m_report_.m_sounding = true;
    publish();
    m_output_.set_sample_rate(frame.m_sample_rate);
    m_output_.write(frame.m_samples);
}

//--------------------------------------------------------------------------------------------------

void Mp3Player::publish()
{
    xQueueOverwrite(m_latest_playback_, &m_report_);
}

}  // namespace avela
