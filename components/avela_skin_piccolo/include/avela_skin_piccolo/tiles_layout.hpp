// Layout of the piccolo skin's tiles (screen selector). Values per screen in layouts/<screen>/tiles.hpp.
#pragma once

#include "avela_skin_piccolo/header_layout.hpp"
#include "avela_ui/canvas.hpp"

namespace avela::skin_piccolo {

// A row of tiles, centred (all of them always visible, so no position dots); under them the footer line and the hint.
struct TilesLayout {
    // An "i" in a circle; offsets from the icon centre.
    struct InfoIcon {
        float m_ring_radius = 0.0f;
        float m_ring_thickness = 0.0f;
        int m_dot_y = 0;
        int m_dot_radius = 0;
        int m_stem_y = 0;  // top of the stem
        int m_stem_width = 0;
        int m_stem_height = 0;
    };

    int m_top = 0;
    int m_side_margin = 0;  // left and right of the row; the tiles share the width between (2 or 3 tiles)
    int m_tile_height = 0;
    int m_gap = 0;
    int m_corner_radius = 0;
    int m_border = 0;  // outline width; the selected tile's is selected_border
    int m_selected_border = 0;
    int m_icon_center_y = 0;   // below the tile's top
    int m_label_center_y = 0;  // below the tile's top
    Point m_hint{};            // centre
    int m_weather_icon_size = 0;
    InfoIcon m_info_icon{};
};

}  // namespace avela::skin_piccolo

#if CONFIG_AVELA_SCREEN_LANDSCAPE_320X170
#include "avela_skin_piccolo/layouts/landscape_320x170/tiles.hpp"
#endif
