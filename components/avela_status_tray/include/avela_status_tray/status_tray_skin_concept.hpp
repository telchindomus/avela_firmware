// What the status tray (the right part of the header) shows, as its skin must take it: the local time, the radio's
// play state and volume, and the Wi-Fi status with its signal bars, as concrete values; the skin formats and draws
// them. Once shown (draw()), each setter redraws at once if a value differs from what it shows (the clock: once a
// minute).
// Skin authors: components/avela_status_tray/docs/developer/SKIN.md describes every setter (values, ranges, controls).
#pragma once

#include <concepts>

#include "avela_clock/local_time.hpp"
#include "avela_network/model/wifi_status.hpp"
#include "avela_radio/model/playback_state.hpp"

namespace avela {

template <typename T>
concept StatusTraySkinConcept = requires(T& skin, const LocalTime& time, bool playing, PlaybackState state, int volume,
                                         WifiStatus status, int bars) {
    { skin.set_time(time) } -> std::same_as<void>;
    { skin.set_playback(playing, state) } -> std::same_as<void>;  // requested play/stop, what the radio does
    { skin.set_volume(volume) } -> std::same_as<void>;            // RadioModel::volume_min..volume_max
    { skin.set_wifi(status, bars) } -> std::same_as<void>;        // bars: NetworkModel::signal_bars(), 0..3
    { skin.draw() } -> std::same_as<void>;
};

}  // namespace avela
