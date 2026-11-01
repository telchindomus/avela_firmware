// What every input provides (GpioButton, PcntEncoder): poll(now_ms), called every few milliseconds, returns the input
// event this poll completes, if any (at most one per poll: an encoder adds up its detents into one Turn, a button
// can't complete two events between polls), with the input as its sender. A concept: compile-time polymorphism only
// (docs/developer/CODING_STANDARD.md, section 4); InputPoller polls a set of them.
#pragma once

#include <concepts>
#include <cstdint>
#include <optional>

#include "avela_hal/input/events/input_event.hpp"

namespace avela {

template <typename T>
concept InputConcept = requires(T& input, uint32_t now_ms) {
    { input.poll(now_ms) } -> std::same_as<std::optional<InputEvent>>;
};

}  // namespace avela
