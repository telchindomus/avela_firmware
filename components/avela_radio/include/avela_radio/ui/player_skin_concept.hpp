// What the player screen shows, as its skin must take it: concrete values, no models. Once the skin is
// shown (draw()), each setter redraws its part at once if the value differs from what it shows; before that it only
// keeps the value. The screen calls every setter each frame.
// Skin authors: components/avela_radio/docs/developer/SKIN.md describes every setter (values, ranges, controls).
#pragma once

#include <concepts>
#include <cstdint>
#include <span>
#include <string_view>

#include "avela_radio/model/playback_state.hpp"
#include "avela_radio/ui/thumbnail.hpp"

namespace avela {

template <typename T>
concept PlayerSkinConcept = requires(T& skin, std::string_view text, const Thumbnail& thumbnail, bool playing,
                                     PlaybackState state, std::span<const uint8_t> spectrum) {
    { skin.set_station(text, text) } -> std::same_as<void>;  // name, genre
    { skin.set_thumbnail(thumbnail) } -> std::same_as<void>;
    { skin.set_stream(text, text) } -> std::same_as<void>;     // title, format ("128 kbit/s MP3")
    { skin.set_playing(playing) } -> std::same_as<void>;       // requested: play (true) or stop
    { skin.set_playback_state(state) } -> std::same_as<void>;  // what the radio does about it
    { skin.set_spectrum(spectrum) } -> std::same_as<void>;  // RadioModel::m_spectrum; the skin decides bars and motion
    { skin.draw() } -> std::same_as<void>;                  // everything, with the values set so far
};

}  // namespace avela
