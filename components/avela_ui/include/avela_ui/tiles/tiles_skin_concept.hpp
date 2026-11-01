// What the tiles (the screen selector) show, as their skin must take it: the screens to offer, in order, and the
// chosen one. Once shown (draw()), set_tiles() redraws at once if anything differs from what it shows; the view calls
// it each frame.
// Skin authors: components/avela_ui/docs/developer/SKIN.md describes it (values, controls).
#pragma once

#include <concepts>
#include <cstddef>
#include <span>

#include "avela_ui/screen_id.hpp"

namespace avela {

template <typename T>
concept TilesSkinConcept = requires(T& skin, std::span<const ScreenId> screens, std::size_t selected) {
    { skin.set_tiles(screens, selected) } -> std::same_as<void>;  // screens: static storage
    { skin.draw() } -> std::same_as<void>;
};

}  // namespace avela
