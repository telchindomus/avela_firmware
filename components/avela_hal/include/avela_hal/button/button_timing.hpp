// How a button tells its input events apart (Click, DoubleClick, LongPress): debounce, long press and double-click
// windows; the timings of the app's two kinds of buttons.
#pragma once

#include <cstdint>

namespace avela {

struct ButtonTiming {
    uint32_t m_debounce_ms;
    uint32_t m_long_press_ms;  // 0 = no long press: a click however long the button is held
    uint32_t
        m_double_click_ms;  // a click is reported only after this, unless a second press comes; 0 = no double click
};

inline constexpr ButtonTiming default_button_timing{
    .m_debounce_ms = 20, .m_long_press_ms = 600, .m_double_click_ms = 300};
// A back button has no double click and no long press: its Click is reported on release, however long it was held.
inline constexpr ButtonTiming back_button_timing{.m_debounce_ms = 20, .m_long_press_ms = 0, .m_double_click_ms = 0};

}  // namespace avela
