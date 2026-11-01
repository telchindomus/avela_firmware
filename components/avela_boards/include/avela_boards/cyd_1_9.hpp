// The CYD ("Cheap Yellow Display") ESP32 1.9" 170x320 (ST7789) module as bought: only what is on it, the display
// and the BOOT button: their description (static constexpr, usable without building anything) and the parts
// themselves. As bought it is a board already (BoardHardware: it polls its BOOT button). A board built on it extends
// it with what is wired to it and names the roles (AvelaPiccolo: the BOOT button is its back button, so it gives the
// button's timing; it polls all its inputs itself). Display pins from the board's example (TFT_eSPI User_Setup.h).
#pragma once

#include <cstdint>
#include <span>

#include "avela_boards/board_hardware_concept.hpp"
#include "avela_hal/board/screen_size.hpp"
#include "avela_hal/button/button.hpp"
#include "avela_hal/button/button_timing.hpp"
#include "avela_hal/button/gpio_button.hpp"
#include "avela_hal/display/st7789_config.hpp"
#include "avela_hal/display/st7789_display.hpp"
#include "avela_hal/input/events/input_event.hpp"
#include "avela_hal/input/input_poller.hpp"
#include "driver/gpio.h"

namespace avela {

class Cyd_1_9 {
public:
    static constexpr St7789Config display_config{
        .m_pins = {.m_sclk = 14,
                   .m_mosi = 13,
                   .m_chip_select = 15,
                   .m_data_command = 2,
                   .m_reset = -1,  // not wired to a GPIO on this board
                   .m_backlight = 21},
        // HSPI on the ESP32; ST7789 is rated for 62.5 MHz writes, 40 MHz is the safe start
        .m_bus = {.m_host = SPI2_HOST, .m_write_hz = 40'000'000, .m_read_hz = 16'000'000},
        // ST7789 frame memory is 240x320; the glass shows a 170 wide window in its middle (IPS: inverted)
        .m_panel = {.m_memory_width = 240,
                    .m_memory_height = 320,
                    .m_width = 170,
                    .m_height = 320,
                    .m_offset_x = (240 - 170) / 2,
                    .m_invert = true},
        .m_backlight = {.m_pwm_hz = 12'000, .m_pwm_channel = 7},  // the last LEDC channel
    };

    static constexpr ScreenSize screen{.m_width = 320, .m_height = 170};  // landscape

    // The BOOT button: active low, pulled up on the board. Held down at reset it starts the chip's flasher instead of
    // the app (the ESP32's own strapping); once the app runs it is an ordinary button.
    static constexpr gpio_num_t boot_button_pin = GPIO_NUM_0;

    // Once: the display and the BOOT button.
    void init();

    // The display (LovyanGFX), for the canvas to draw on; set up by init().
    lgfx::LGFX_Device& display() { return m_display_; }

    // What the BOOT button did since the last poll, with its sender; valid until the next poll.
    std::span<const InputEvent> poll_inputs(uint32_t now_ms) { return m_inputs_.poll(now_ms); }

    // The BOOT button as the sender of its input events: compare with an event's m_sender.
    [[nodiscard]] const Button& boot_button() const { return m_boot_button_; }

protected:
    // boot_button_timing: what the board uses the BOOT button for
    explicit Cyd_1_9(ButtonTiming boot_button_timing) : m_boot_button_(boot_button_pin, boot_button_timing) {}

    // The BOOT button as an input, for the board's InputPoller.
    GpioButton& boot_button_input() { return m_boot_button_; }

private:
    St7789Display m_display_{display_config};
    GpioButton m_boot_button_;
    InputPoller<GpioButton> m_inputs_{m_boot_button_};
};

static_assert(BoardHardwareConcept<Cyd_1_9>);

}  // namespace avela
