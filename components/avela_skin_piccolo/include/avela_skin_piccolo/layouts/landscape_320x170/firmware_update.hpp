// The firmware update screen on the 320x170 landscape screen (CYD 1.9").
// Included by firmware_update_layout.hpp only, after the layout structs.
#pragma once

namespace avela::skin_piccolo::layout {

inline constexpr FirmwareUpdateLayout firmware_update{
    .m_badge = {.m_x = 10, .m_y = 35, .m_width = 49, .m_height = 13},
    .m_headline = {.m_x = 10, .m_y = 52},
    .m_status_line = {.m_x = 10, .m_y = 79},
    .m_rows_top = 98,
    .m_row_height = 17,
    .m_label_x = 10,
    .m_value_x = 50,
    .m_progress = {.m_x = 10, .m_y = 136, .m_width = 300, .m_height = 6},
    .m_state = {.m_x = 10, .m_y = 150},
    .m_hint = {.m_x = 310, .m_y = 150},
};

}  // namespace avela::skin_piccolo::layout
