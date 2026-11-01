// Layout of the Silk Taupe skin's station list. Values per screen in layouts/<screen>/station_list.hpp.
#pragma once

#include "avela_skin_piccolo_silk_taupe/header_layout.hpp"
#include "avela_ui/canvas.hpp"

namespace avela::skin_piccolo_silk_taupe {

// The selected station in a highlighted row, its neighbours above and below, a hint line at the bottom.
struct StationListLayout {
    Rect m_area{};  // between the header and footer lines; redrawn when the selection moves
    int m_previous_center_y = 0;
    int m_next_center_y = 0;
    int m_neighbour_text_x = 0;
    int m_current_marker_x = 0;  // centre of the triangle in front of a neighbour that is playing
    int m_current_marker_radius = 0;
    int m_current_marker_gap = 0;  // in the selected row: from the triangle's right tip to the position
    Rect m_selected_row{};
    int m_row_corner_radius = 0;
    int m_accent_inset = 0;  // gold bar on the row's left side: inset from the row's top and bottom
    int m_accent_width = 0;
    int m_accent_corner_radius = 0;
    int m_text_clip_inset = 0;  // name and genre are cut text_clip_inset + text_clip_width from the row's left edge
    int m_text_clip_width = 0;
    Point m_name_offset{};  // from the row's top left corner
    Point m_genre_offset{};
    int m_position_right_inset = 0;  // "03 / 05": right edge, from the row's right edge
    Point m_hint{};                  // centre of the hint line
};

}  // namespace avela::skin_piccolo_silk_taupe

#if CONFIG_AVELA_SCREEN_LANDSCAPE_320X170
#include "avela_skin_piccolo_silk_taupe/layouts/landscape_320x170/station_list.hpp"
#endif
