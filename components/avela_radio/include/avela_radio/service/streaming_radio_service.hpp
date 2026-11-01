// The radio for real: plays the requested station's MP3 stream through the board's amplifier while the radio is on
// the home network and play is requested. Two tasks do the work (StreamReceiver: the network side; Mp3Player:
// decoding and sound); update() on the main loop tells them what the model asks for (station, play/pause, volume) and
// writes back what they report (playback state, stream title, format, spectrum). They talk through queues only.
#pragma once

#include <cstdint>
#include <string_view>

#include "avela_hal/audio/i2s_output.hpp"
#include "avela_network/model/network_model.hpp"
#include "avela_radio/model/station_record.hpp"
#include "avela_radio/service/mp3_player.hpp"
#include "avela_radio/service/player_queues.hpp"
#include "avela_radio/service/radio_service_concept.hpp"
#include "avela_radio/service/stream_receiver.hpp"
#include "freertos/FreeRTOS.h"

namespace avela {

class StreamingRadioService {
public:
    // network: read in update() to know whether the home network is joined; output: the board's amplifier
    StreamingRadioService(const NetworkModel& network, I2sOutput& output)
        : m_network_(network), m_player_(output, m_receiver_)
    {}

    // Starts the tasks (stopped until the first update() asks for a station).
    void init();
    void update(RadioModel& model);
    // The queues its clients send requests to and read the status from.
    PlayerQueues& queues() { return m_queues_; }

private:
    static constexpr int volume_unset = RadioModel::volume_min - 1;

    // Tunes, stops and sets the volume when the model's requests changed.
    void follow_requests(const RadioModel& model);
    // The current station's stream address (from its StationRecord); empty if its slot holds none.
    std::string_view station_url(const RadioModel& model);
    // The tasks' newest reports, into the model.
    void report(RadioModel& model);
    [[nodiscard]] PlaybackState playback_state() const;
    // While a station is tuned, every 10 s: heap, queue fill, dropouts, the tasks' stacks (to find why sound stops).
    void log_diagnostics();

    PlayerQueues m_queues_;
    const NetworkModel& m_network_;
    StreamReceiver m_receiver_;
    Mp3Player m_player_;

    TuneRequest m_tuned_;  // what the receiver was asked for last
    uint8_t m_url_slot_ = 0;
    uint32_t m_url_revision_ = 0;
    FixedString<StationRecord::url_capacity> m_station_url_;  // of m_url_slot_
    int m_volume_ = volume_unset;
    ReceptionReport m_reception_;
    PlaybackReport m_playback_;
    TickType_t m_diagnostics_logged_ = 0;
};

static_assert(RadioServiceConcept<StreamingRadioService>);

}  // namespace avela
