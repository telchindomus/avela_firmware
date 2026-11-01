// The board's own settings, as saved by the setup page's board section (a Settings record, avela_settings): the
// screen's brightness. The app applies them on the main loop (Application::apply_board_settings).
#pragma once

#include <cstdint>

namespace avela {

struct BoardSettings {
    static constexpr const char* key = "board";
    static constexpr uint16_t version = 1;
    static constexpr int brightness_min = 10;  // percent; below it the screen is hard to read
    static constexpr int brightness_max = 100;
    static constexpr uint8_t default_brightness = 80;

    uint8_t m_brightness_percent = default_brightness;
};

// The backlight's PWM level (0..255) for a brightness in percent.
constexpr uint8_t backlight_level(int brightness_percent)
{
    constexpr int full_level = 255;
    return static_cast<uint8_t>(brightness_percent * full_level / BoardSettings::brightness_max);
}

}  // namespace avela
