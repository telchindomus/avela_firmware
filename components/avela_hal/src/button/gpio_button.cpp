#include "avela_hal/button/gpio_button.hpp"

#include "esp_check.h"

namespace avela {

//--------------------------------------------------------------------------------------------------

void GpioButton::init()
{
    gpio_config_t config{};
    config.pin_bit_mask = 1ULL << m_pin_;
    config.mode = GPIO_MODE_INPUT;
    config.pull_up_en = GPIO_PULLUP_ENABLE;
    ESP_ERROR_CHECK(gpio_config(&config));
}

//--------------------------------------------------------------------------------------------------

std::optional<InputEvent> GpioButton::poll(uint32_t now_ms)
{
    return m_state_machine_.update(gpio_get_level(m_pin_) == 0, now_ms, *this);
}

}  // namespace avela
