// How a board's ST7789 panel is wired and set up (St7789Display): SPI pins and clocks, the panel's frame memory and the
// window of it the glass shows, the PWM backlight. Field types as LovyanGFX's own settings.
#pragma once

#include <cstdint>

#include "driver/spi_common.h"

namespace avela {

struct St7789Config {
    struct Pins {
        int16_t m_sclk;
        int16_t m_mosi;
        int16_t m_chip_select;
        int16_t m_data_command;
        int16_t m_reset;  // -1 = not wired to a GPIO
        int16_t m_backlight;
    };
    struct Bus {
        spi_host_device_t m_host;
        uint32_t m_write_hz;
        uint32_t m_read_hz;
    };
    // The panel's frame memory and the window of it the glass shows.
    struct Panel {
        uint16_t m_memory_width;
        uint16_t m_memory_height;
        uint16_t m_width;
        uint16_t m_height;
        uint16_t m_offset_x;
        bool m_invert;  // IPS panels
    };
    struct Backlight {
        uint32_t m_pwm_hz;
        uint8_t m_pwm_channel;  // LEDC channel
    };

    Pins m_pins;
    Bus m_bus;
    Panel m_panel;
    Backlight m_backlight;
};

}  // namespace avela
