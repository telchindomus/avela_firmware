// A push button's input events from its debounced level: Click / DoubleClick / LongPress (avela_hal/input/events/).
// Plain state machine (no GPIO, platform-free): the button that owns the pin (GpioButton) feeds it the raw level
// every poll and is the events' sender.
#pragma once

#include <cstdint>
#include <optional>

#include "avela_hal/button/button.hpp"
#include "avela_hal/button/button_timing.hpp"
#include "avela_hal/input/events/input_event.hpp"

namespace avela {

class ButtonStateMachine {
public:
    explicit ButtonStateMachine(ButtonTiming timing) : m_timing_(timing) {}
    // The event this poll completes, if any, sent by `sender`.
    std::optional<InputEvent> update(bool raw_pressed, uint32_t now_ms, const Button& sender);

private:
    enum class State : uint8_t { idle, pressed, waiting_for_second_press, second_press, long_press_held };

    ButtonTiming m_timing_;
    State m_state_ = State::idle;
    bool m_raw_pressed_ = false;
    bool m_stable_pressed_ = false;
    uint32_t m_raw_changed_ms_ = 0;
    uint32_t m_state_since_ms_ = 0;
};

}  // namespace avela
