// What the radio does now, for the setup page and the console (through the radio service's StatusQueue): a snapshot
// of the RadioModel: the station (slot and name), play or stop, the volume and the playback state. Platform-free;
// trivially copyable (it crosses tasks in a queue).
#pragma once

#include <cstdint>

#include "avela_radio/model/playback_state.hpp"
#include "avela_radio/model/radio_model.hpp"
#include "avela_radio/model/station_record.hpp"
#include "avela_strings/fixed_string.hpp"

namespace avela {

struct PlayerStatus {
    uint8_t m_station_slot = 0;
    FixedString<StationRecord::name_capacity> m_station_name;
    bool m_playing = false;
    int m_volume = 0;
    PlaybackState m_state = PlaybackState::stopped;

    bool operator==(const PlayerStatus&) const = default;
};

// The status as the model holds it.
PlayerStatus status_of(const RadioModel& model);

}  // namespace avela
