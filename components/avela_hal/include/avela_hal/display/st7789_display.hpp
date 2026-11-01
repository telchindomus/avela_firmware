// A LovyanGFX device for an ST7789 panel on an SPI bus with a PWM backlight, set up from a board's St7789Config.
// LovyanGFX does the driving; this only fills in its bus / panel / backlight settings. The UI draws through the
// lgfx::LGFX_Device base, so it never sees which panel it is.
#pragma once

#include <LovyanGFX.hpp>

#include "avela_hal/display/st7789_config.hpp"

namespace avela {

class St7789Display : public lgfx::LGFX_Device {
public:
    explicit St7789Display(const St7789Config& config);

private:
    lgfx::Panel_ST7789 m_panel_;
    lgfx::Bus_SPI m_bus_;
    lgfx::Light_PWM m_backlight_;
};

}  // namespace avela
