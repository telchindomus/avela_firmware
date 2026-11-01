// Layout of the pieces every skin of the piccolo skin shares: the header band, the content area below it and the line
// above the bottom band. Values per screen in layouts/<screen>/header.hpp.
#pragma once

#include "avela_skin_piccolo/screen.hpp"
#include "avela_ui/canvas.hpp"

namespace avela::skin_piccolo {

// Logo and brand name on the left, the tray (clock, date, status icons) on the right, the divider line below.
struct HeaderLayout {
    Rect m_area{};  // redrawn when the tray changes
    int m_divider_y = 0;
    Point m_logo{};           // top left corner
    int m_brand_gap = 0;      // logo's right edge -> brand name
    int m_text_center_y = 0;  // brand name and tray are centred on this line
    int m_tray_right = 0;     // the tray's right edge
};

}  // namespace avela::skin_piccolo

#if CONFIG_AVELA_SCREEN_LANDSCAPE_320X170
#include "avela_skin_piccolo/layouts/landscape_320x170/header.hpp"
#endif
