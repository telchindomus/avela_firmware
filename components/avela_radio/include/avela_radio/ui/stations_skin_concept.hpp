// What the station list shows, as its skin must take it: the stations, the selected one and the one
// playing. The skin decides how many rows around the selection it shows. Once shown (draw()), set_selection()
// redraws at once if anything differs from what it shows; the screen calls it each frame.
// Skin authors: components/avela_radio/docs/developer/SKIN.md describes every setter (values, ranges, controls).
#pragma once

#include <concepts>
#include <cstddef>
#include <span>

#include "avela_radio/model/station.hpp"

namespace avela {

template <typename T>
concept StationsSkinConcept = requires(T& skin, std::span<const Station> stations, std::size_t index) {
    { skin.set_selection(stations, index, index) } -> std::same_as<void>;  // stations, selected, playing
    { skin.draw() } -> std::same_as<void>;
};

}  // namespace avela
