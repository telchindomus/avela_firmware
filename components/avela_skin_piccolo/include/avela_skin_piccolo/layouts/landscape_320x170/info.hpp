// The info screen on the 320x170 landscape screen (CYD 1.9").
// Included by info_layout.hpp only, after the layout structs.
#pragma once

namespace avela::skin_piccolo::layout {

inline constexpr InfoLayout info{
    .m_badge = {.m_x = 10, .m_y = 35, .m_width = 33, .m_height = 13},
    .m_details_area = {.m_x = 0, .m_y = 52, .m_width = screen.m_width, .m_height = screen.m_height - 52},
    .m_rows_top = 56,
    .m_row_height = 17,
    .m_label_x = 10,
    .m_value_x = 72,
    .m_ring_center = {.m_x = 284, .m_y = 126},
    .m_ring_radius = 22.0f,
    .m_ring_thickness = 3.0f,
    .m_ring_caption = {.m_x = 284, .m_y = 158},
    .m_hint = {.m_x = 10, .m_y = 150},
};

}  // namespace avela::skin_piccolo::layout
