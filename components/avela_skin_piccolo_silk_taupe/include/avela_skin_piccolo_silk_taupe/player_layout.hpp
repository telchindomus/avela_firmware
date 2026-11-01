// Layout of the Silk Taupe skin's player. Values per screen in layouts/<screen>/player.hpp.
#pragma once

#include "avela_skin_piccolo_silk_taupe/header_layout.hpp"
#include "avela_ui/canvas.hpp"

namespace avela::skin_piccolo_silk_taupe {

struct PlayerLayout {
    // Rounded square with a ring and the station's monogram (the thumbnail).
    struct StationTile {
        Rect m_area{};
        int m_corner_radius = 0;
        float m_ring_radius = 0.0f;
        float m_ring_thickness = 0.0f;
        int m_monogram_drop = 0;  // monogram centre below the tile centre (optical centring)
    };

    // Bars spread evenly from left to right, growing up from the bottom line; over each bar its peak, a short cap that
    // stays a moment at the highest level and then sinks.
    struct Spectrum {
        Rect m_area{};  // redrawn while the bars move
        int m_bar_count = 0;
        int m_left = 0;    // left edge of the first bar
        int m_right = 0;   // right edge of the last bar
        int m_bottom = 0;  // bars grow up from here
        int m_bar_width = 0;
        int m_bar_min_height = 0;  // silence
        int m_bar_max_height = 0;
        int m_bar_corner_radius = 0;
        int m_peak_height = 0;
        int m_peak_gap = 0;  // between a bar's top and its peak
    };

    StationTile m_station_tile{};
    Rect m_text_area{};       // format, station name, stream title and genre; redrawn when one of them changes
    Point m_stream_format{};  // left edge, vertical centre
    Rect m_text_clip{};       // station name, stream title and genre are cut at its right edge
    Point m_station_name{};   // top left corner of each text line
    Point m_stream_title{};
    Point m_genre{};
    Spectrum m_spectrum{};
};

}  // namespace avela::skin_piccolo_silk_taupe

#if CONFIG_AVELA_SCREEN_LANDSCAPE_320X170
#include "avela_skin_piccolo_silk_taupe/layouts/landscape_320x170/player.hpp"
#endif
