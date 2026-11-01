// A request to the radio: the station (its slot), play or stop (or the other of the two), the volume (or a change of
// it); what isn't given stays as it is. Changes rather than values where a request is made from what the sender saw
// (the knob: one more step, play/stop the other way), so requests queued before the radio carries them out add up.
// Sent by the knob's controllers, the setup page and the console into the radio's queue.
#pragma once

#include <cstdint>
#include <optional>

namespace avela {

struct PlayerRequest {
    uint8_t m_station_slot = 0;  // 1..16; 0 = keep the station (a slot without one too)
    std::optional<bool> m_playing = std::nullopt;
    bool m_toggle_playing = false;               // after m_playing
    std::optional<int> m_volume = std::nullopt;  // 0..100 (clamped)
    int m_volume_change = 0;                     // after m_volume, then clamped
};

}  // namespace avela
