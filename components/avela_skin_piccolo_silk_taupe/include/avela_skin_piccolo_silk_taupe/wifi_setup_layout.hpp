// Layout of the Silk Taupe skin's Wi-Fi setup screen. Values per screen in layouts/<screen>/wifi_setup.hpp.
#pragma once

#include "avela_skin_piccolo_silk_taupe/header_layout.hpp"
#include "avela_ui/canvas.hpp"

namespace avela::skin_piccolo_silk_taupe {

// Badge, headline and status line at the top left, label/value rows below, the hint at the bottom, the QR code to
// join the setup network on the right.
struct WifiSetupLayout {
    Rect m_badge{};
    Point m_headline{};     // top left corner
    Point m_status_line{};  // top left corner
    int m_rows_top = 0;
    int m_row_height = 0;
    int m_label_x = 0;
    int m_value_x = 0;
    Point m_hint{};              // top left corner
    Rect m_qr_code{};            // square: palette::qr_dark modules on a palette::qr_light square
    int m_qr_corner_radius = 0;  // the light square's corners (kept within its margin)
    Point m_qr_caption{};        // centre of the caption under it
};

}  // namespace avela::skin_piccolo_silk_taupe

#if CONFIG_AVELA_SCREEN_LANDSCAPE_320X170
#include "avela_skin_piccolo_silk_taupe/layouts/landscape_320x170/wifi_setup.hpp"
#endif
