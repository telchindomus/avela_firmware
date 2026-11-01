// The player on the 320x170 landscape screen (CYD 1.9").
// Included by player_layout.hpp only, after the layout structs.
#pragma once

namespace avela::skin_piccolo_silk_taupe::layout {

inline constexpr PlayerLayout player{
    .m_station_tile =
        {
            .m_area = {.m_x = 10, .m_y = 36, .m_width = 58, .m_height = 58},
            .m_corner_radius = 6,
            .m_ring_radius = 19.5f,
            .m_ring_thickness = 1.4f,
            .m_monogram_drop = 1,
        },
    .m_text_area = {.m_x = 70, .m_y = 30, .m_width = 250, .m_height = 74},
    .m_stream_format = {.m_x = 78, .m_y = 41},
    .m_text_clip = {.m_x = 78, .m_y = 48, .m_width = 232, .m_height = 56},
    .m_station_name = {.m_x = 78, .m_y = 51},
    .m_stream_title = {.m_x = 78, .m_y = 74},
    .m_genre = {.m_x = 78, .m_y = 90},
    .m_spectrum =
        {
            .m_area = {.m_x = 8, .m_y = 106, .m_width = 304, .m_height = 62},
            .m_bar_count = 32,
            .m_left = 10,
            .m_right = 310,
            .m_bottom = 165,
            .m_bar_width = 6,
            .m_bar_min_height = 2,
            .m_bar_max_height = 50,
            .m_bar_corner_radius = 2,
            .m_peak_height = 2,
            .m_peak_gap = 2,
        },
};

}  // namespace avela::skin_piccolo_silk_taupe::layout
