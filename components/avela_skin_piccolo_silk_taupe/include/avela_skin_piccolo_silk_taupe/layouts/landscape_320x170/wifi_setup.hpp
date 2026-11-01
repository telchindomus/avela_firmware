// The Wi-Fi setup screen on the 320x170 landscape screen (CYD 1.9").
// Included by wifi_setup_layout.hpp only, after the layout structs.
#pragma once

namespace avela::skin_piccolo_silk_taupe::layout {

inline constexpr WifiSetupLayout wifi_setup{
    .m_badge = {.m_x = 10, .m_y = 35, .m_width = 41, .m_height = 13},
    .m_headline = {.m_x = 10, .m_y = 52},
    .m_status_line = {.m_x = 10, .m_y = 79},
    .m_rows_top = 100,
    .m_row_height = 17,
    .m_label_x = 10,
    .m_value_x = 76,
    .m_hint = {.m_x = 10, .m_y = 145},
    .m_qr_code = {.m_x = 204, .m_y = 36, .m_width = 106, .m_height = 106},
    .m_qr_corner_radius = 8,  // like the tiles
    .m_qr_caption = {.m_x = 257, .m_y = 155},
};

}  // namespace avela::skin_piccolo_silk_taupe::layout
