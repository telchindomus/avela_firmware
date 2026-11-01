// The tiles screen on the 320x170 landscape screen (CYD 1.9").
// Included by tiles_layout.hpp only, after the layout structs.
#pragma once

namespace avela::skin_piccolo::layout {

inline constexpr TilesLayout tiles{
    .m_top = 38,
    .m_side_margin = 12,
    .m_tile_height = 80,
    .m_gap = 10,
    .m_corner_radius = 8,
    .m_border = 1,
    .m_selected_border = 2,
    .m_icon_center_y = 32,
    .m_label_center_y = 64,
    .m_hint = {.m_x = screen.m_width / 2, .m_y = 152},
    .m_weather_icon_size = 30,
    .m_info_icon = {.m_ring_radius = 12.0f,
                    .m_ring_thickness = 2.2f,
                    .m_dot_y = -6,
                    .m_dot_radius = 2,
                    .m_stem_y = -2,
                    .m_stem_width = 2,
                    .m_stem_height = 9},
};

}  // namespace avela::skin_piccolo::layout
