// What an input did, and which input it was (each event's m_sender: a Button or an Encoder): Turn from an encoder,
// Click / DoubleClick / LongPress from a button. No meaning: what an event does is up to its receiver.
#pragma once

#include <variant>

#include "avela_hal/input/events/click.hpp"
#include "avela_hal/input/events/double_click.hpp"
#include "avela_hal/input/events/long_press.hpp"
#include "avela_hal/input/events/turn.hpp"

namespace avela {

using InputEvent = std::variant<Turn, Click, DoubleClick, LongPress>;

}  // namespace avela
