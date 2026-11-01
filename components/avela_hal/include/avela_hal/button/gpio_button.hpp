// A push button on a GPIO, active low (pulled up, pressed = 0): a Button whose events come from its
// ButtonStateMachine, with itself as their sender.
#pragma once

#include <cstdint>
#include <optional>

#include "avela_hal/button/button.hpp"
#include "avela_hal/button/button_state_machine.hpp"
#include "avela_hal/button/button_timing.hpp"
#include "avela_hal/input/input_concept.hpp"
#include "driver/gpio.h"

namespace avela {

class GpioButton : public Button {
public:
    GpioButton(gpio_num_t pin, ButtonTiming timing) : m_pin_(pin), m_state_machine_(timing) {}

    void init();
    std::optional<InputEvent> poll(uint32_t now_ms);

private:
    gpio_num_t m_pin_;
    ButtonStateMachine m_state_machine_;
};

static_assert(InputConcept<GpioButton>);

}  // namespace avela
