#include "avela_radio/service/streaming_radio_service.hpp"

#include <cinttypes>
#include <string_view>

#include "avela_network/model/wifi_status.hpp"
#include "avela_radio/model/station_record.hpp"
#include "avela_settings/settings.hpp"
#include "esp_heap_caps.h"
#include "esp_log.h"

namespace avela {
namespace {

constexpr const char* log_tag = "radio";
constexpr TickType_t diagnostics_interval = pdMS_TO_TICKS(10000);
constexpr std::size_t bytes_per_kilobyte = 1024;

}  // namespace

//--------------------------------------------------------------------------------------------------

void StreamingRadioService::init()
{
    m_receiver_.init();
    m_player_.init();
}

//--------------------------------------------------------------------------------------------------

void StreamingRadioService::update(RadioModel& model)
{
    m_queues_.take_requests(model);
    follow_requests(model);
    report(model);
    m_queues_.publish(model);
    log_diagnostics();
}

//--------------------------------------------------------------------------------------------------

void StreamingRadioService::follow_requests(const RadioModel& model)
{
    const bool joined = m_network_.m_status == WifiStatus::connected;
    const std::string_view wanted = model.m_playing && joined ? station_url(model) : "";
    if (wanted != m_tuned_.m_url.view()) {
        m_tuned_.m_url.clear().append(wanted);
        m_receiver_.tune(wanted);
        // until the receiver's first report about it: the old station's reports no longer count
        m_reception_ = ReceptionReport{};
        m_reception_.m_status = wanted.empty() ? ReceptionStatus::idle : ReceptionStatus::connecting;
    }
    if (model.m_volume != m_volume_) {
        m_volume_ = model.m_volume;
        m_player_.set_volume(m_volume_);
    }
}

//--------------------------------------------------------------------------------------------------

// The address is read from the station's record when the station or the saved stations change, not every frame.
std::string_view StreamingRadioService::station_url(const RadioModel& model)
{
    const uint8_t slot = model.m_stations.slot(model.m_station_index);
    const uint32_t revision = Settings::revision<StationRecord>();
    if (slot != m_url_slot_ || revision != m_url_revision_) {
        m_url_slot_ = slot;
        m_url_revision_ = revision;
        StationRecord record;
        m_station_url_.clear();
        if (Settings::load(record, slot)) {
            m_station_url_.append(record.m_url.view());
        }
    }
    return m_station_url_.view();
}

//--------------------------------------------------------------------------------------------------

void StreamingRadioService::report(RadioModel& model)
{
    m_receiver_.take_report(m_reception_);
    m_player_.take_report(m_playback_);
    const PlaybackState state = playback_state();
    const bool playing = state == PlaybackState::playing;
    model.m_playback_state = state;
    if (m_reception_.m_status == ReceptionStatus::receiving) {
        model.m_stream_title.clear().append(m_reception_.m_title.view());
    } else {
        model.m_stream_title.clear();
    }
    model.m_stream_format.clear().append(playing ? m_playback_.m_format.view() : "");
    if (playing) {
        model.m_spectrum = m_playback_.m_spectrum;
    } else {
        model.m_spectrum.fill(0);
    }
}

//--------------------------------------------------------------------------------------------------

void StreamingRadioService::log_diagnostics()
{
    const TickType_t now = xTaskGetTickCount();
    if (m_tuned_.m_url.empty() || now - m_diagnostics_logged_ < diagnostics_interval) {
        return;
    }
    m_diagnostics_logged_ = now;
    ESP_LOGI(log_tag,
             "heap %zu KB free, %zu KB lowest, %zu KB largest block; queue %u/%zu; dropouts %" PRIu32
             "; stack left: stream %" PRIu32 " B, player %" PRIu32 " B",
             heap_caps_get_free_size(MALLOC_CAP_8BIT) / bytes_per_kilobyte,
             heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT) / bytes_per_kilobyte,
             heap_caps_get_largest_free_block(MALLOC_CAP_8BIT) / bytes_per_kilobyte,
             uxQueueMessagesWaiting(m_receiver_.encoded_audio()), StreamReceiver::chunk_count, m_player_.dropouts(),
             m_receiver_.stack_left(), m_player_.stack_left());
}

//--------------------------------------------------------------------------------------------------

PlaybackState StreamingRadioService::playback_state() const
{
    if (m_tuned_.m_url.empty()) {
        return PlaybackState::stopped;
    }
    switch (m_reception_.m_status) {
    case ReceptionStatus::idle:
    case ReceptionStatus::connecting:
        return PlaybackState::connecting;
    case ReceptionStatus::failed:
        return PlaybackState::failed;
    case ReceptionStatus::receiving:
        break;
    }
    const bool sounding = m_playback_.m_sounding && m_playback_.m_session == m_reception_.m_session;
    return sounding ? PlaybackState::playing : PlaybackState::connecting;
}

}  // namespace avela
