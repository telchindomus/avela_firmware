#include "avela_hal/button/button_state_machine.hpp"

namespace avela {

//--------------------------------------------------------------------------------------------------

std::optional<InputEvent> ButtonStateMachine::update(bool raw_pressed, uint32_t now_ms, const Button& sender)
{
    if (raw_pressed != m_raw_pressed_) {
        m_raw_pressed_ = raw_pressed;
        m_raw_changed_ms_ = now_ms;
    }
    const bool stable_changed =
        m_raw_pressed_ != m_stable_pressed_ && now_ms - m_raw_changed_ms_ >= m_timing_.m_debounce_ms;
    if (stable_changed) {
        m_stable_pressed_ = m_raw_pressed_;
    }
    const uint32_t elapsed = now_ms - m_state_since_ms_;
    auto enter = [&](State state) {
        m_state_ = state;
        m_state_since_ms_ = now_ms;
    };

    switch (m_state_) {
    case State::idle:
        if (stable_changed && m_stable_pressed_) {
            enter(State::pressed);
        }
        return std::nullopt;
    case State::pressed:
        if (stable_changed && !m_stable_pressed_) {
            enter(State::waiting_for_second_press);
        } else if (m_timing_.m_long_press_ms != 0 && elapsed >= m_timing_.m_long_press_ms) {
            enter(State::long_press_held);
            return LongPress{.m_sender = &sender};
        }
        return std::nullopt;
    case State::waiting_for_second_press:
        if (stable_changed && m_stable_pressed_) {
            enter(State::second_press);
        } else if (elapsed >= m_timing_.m_double_click_ms) {
            enter(State::idle);
            return Click{.m_sender = &sender};
        }
        return std::nullopt;
    case State::second_press:
        if (stable_changed && !m_stable_pressed_) {
            enter(State::idle);
            return DoubleClick{.m_sender = &sender};
        }
        return std::nullopt;
    case State::long_press_held:
        if (stable_changed && !m_stable_pressed_) {
            enter(State::idle);
        }
        return std::nullopt;
    }
    return std::nullopt;
}

}  // namespace avela
