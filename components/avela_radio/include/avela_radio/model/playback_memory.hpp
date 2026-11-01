// What the radio was doing, kept across restarts (a Settings record): the station's slot, the volume and whether it
// was playing (a radio switched off while paused starts paused). Saved by RadioStorage a while after the last change.
#pragma once

#include <cstdint>

namespace avela {

struct PlaybackMemory {
    static constexpr const char* key = "playback";
    static constexpr uint16_t version = 1;

    uint8_t m_station_slot = 0;  // 0 = none saved yet
    int8_t m_volume = 0;
    bool m_playing = false;

    bool operator==(const PlaybackMemory&) const = default;
};

}  // namespace avela
