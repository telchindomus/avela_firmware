// The station list on the 320x170 landscape screen (CYD 1.9").
// Included by station_list_layout.hpp only, after the layout structs.
#pragma once

namespace avela::skin_piccolo::layout {

inline constexpr StationListLayout station_list{
    .m_area = {.m_x = 0,
               .m_y = header.m_divider_y + 1,
               .m_width = screen.m_width,
               .m_height = footer_divider_y - header.m_divider_y - 1},
    .m_previous_center_y = 45,
    .m_next_center_y = 117,
    .m_neighbour_text_x = 22,
    .m_current_marker_x = 13,
    .m_current_marker_radius = 2,
    .m_current_marker_gap = 5,
    .m_selected_row = {.m_x = 8, .m_y = 62, .m_width = 304, .m_height = 38},
    .m_row_corner_radius = 7,
    .m_accent_inset = 7,
    .m_accent_width = 3,
    .m_accent_corner_radius = 1,
    .m_text_clip_inset = 12,
    .m_text_clip_width = 220,
    .m_name_offset = {.m_x = 14, .m_y = 5},
    .m_genre_offset = {.m_x = 14, .m_y = 23},
    .m_position_right_inset = 10,
    .m_hint = {.m_x = screen.m_width / 2, .m_y = 152},
};

}  // namespace avela::skin_piccolo::layout
