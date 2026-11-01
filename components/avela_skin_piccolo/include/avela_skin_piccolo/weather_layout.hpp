// Layout of the piccolo skin's weather screen. Values per screen in layouts/<screen>/weather.hpp.
#pragma once

#include "avela_skin_piccolo/header_layout.hpp"
#include "avela_ui/canvas.hpp"

namespace avela::skin_piccolo {

// The big icon and temperature on the left, the condition and "feels / wind" under them, today's
// high / low and the place in a column, the humidity ring on the right, the next days below the footer line. Without
// values: a headline and a hint instead.
struct WeatherLayout {
    // The forecast row under the footer line: one cell per day with its weekday, a small icon and the high.
    struct ForecastRow {
        int m_left = 0;
        int m_cell_width = 0;
        int m_center_y = 0;
        int m_weekday_x = 0;  // from the cell's left edge
        int m_icon_x = 0;     // icon centre, from the cell's left edge
        int m_icon_size = 0;
        int m_temperature_right = 0;  // from the cell's left edge
    };

    Point m_icon_center{};
    int m_icon_size = 0;
    Point m_temperature{};  // top left of the big number
    Point m_condition{};    // left edge, vertical centre
    Point m_details{};      // left edge, vertical centre
    int m_column_x = 0;     // high, low, place
    int m_high_y = 0;
    int m_low_y = 0;
    int m_place_y = 0;
    int m_column_clip_right = 0;  // the place is cut here, before the ring
    Point m_ring_center{};
    float m_ring_radius = 0.0f;
    float m_ring_thickness = 0.0f;
    Point m_ring_caption{};  // centre of the caption under the ring
    ForecastRow m_forecast{};
    Point m_message{};       // left edge, vertical centre of the headline without values
    Point m_message_hint{};  // left edge, vertical centre of the hint under it
};

}  // namespace avela::skin_piccolo

#if CONFIG_AVELA_SCREEN_LANDSCAPE_320X170
#include "avela_skin_piccolo/layouts/landscape_320x170/weather.hpp"
#endif
