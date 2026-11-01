// Layout of the Silk Taupe skin's info screen. Values per screen in layouts/<screen>/info.hpp.
#pragma once

#include "avela_skin_piccolo_silk_taupe/header_layout.hpp"
#include "avela_ui/canvas.hpp"

namespace avela::skin_piccolo_silk_taupe {

// Badge at the top; label/value rows, a free-RAM ring on the right and the hint at the bottom below it.
struct InfoLayout {
    Rect m_badge{};
    Rect m_details_area{};  // everything under the badge, redrawn every second
    int m_rows_top = 0;
    int m_row_height = 0;
    int m_label_x = 0;
    int m_value_x = 0;
    Point m_ring_center{};
    float m_ring_radius = 0.0f;
    float m_ring_thickness = 0.0f;
    Point m_ring_caption{};  // centre of the caption under the ring
    Point m_hint{};          // top left corner
};

}  // namespace avela::skin_piccolo_silk_taupe

#if CONFIG_AVELA_SCREEN_LANDSCAPE_320X170
#include "avela_skin_piccolo_silk_taupe/layouts/landscape_320x170/info.hpp"
#endif
