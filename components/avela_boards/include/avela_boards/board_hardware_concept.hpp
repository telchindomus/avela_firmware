// What every board has (avela_boards: a module as bought, or a board built on one): its description (static
// constexpr: the screen after the rotation, usable at compile time) and its parts, which the board owns and sets up
// in init(): the display to draw on, and its inputs, whatever they are, polled into input events with their senders
// (no meaning: the application gives them one). Which controls a board has and its sound are not part of it: that
// is AvelaBoard's (avela_board.hpp).
#pragma once

#include <concepts>
#include <cstdint>
#include <span>

#include <LovyanGFX.hpp>

#include "avela_hal/board/screen_size.hpp"
#include "avela_hal/input/events/input_event.hpp"

namespace avela {

template <typename T>
concept BoardHardwareConcept = requires(T& board, uint32_t now_ms) {
    { T::screen } -> std::convertible_to<ScreenSize>;  // after the display's rotation
    { board.init() } -> std::same_as<void>;
    { board.display() } -> std::same_as<lgfx::LGFX_Device&>;
    { board.poll_inputs(now_ms) } -> std::same_as<std::span<const InputEvent>>;
};

}  // namespace avela
