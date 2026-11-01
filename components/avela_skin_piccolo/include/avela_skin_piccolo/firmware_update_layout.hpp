// Layout of the piccolo skin's firmware update screen. Values per screen in layouts/<screen>/firmware_update.hpp.
#pragma once

#include "avela_skin_piccolo/header_layout.hpp"
#include "avela_ui/canvas.hpp"

namespace avela::skin_piccolo {

// Badge, headline and the running version at the top left, the update page's addresses as label/value rows below,
// then the progress bar, and under it the update's state on the left and the back hint on the right.
struct FirmwareUpdateLayout {
    Rect m_badge{};
    Point m_headline{};     // top left corner
    Point m_status_line{};  // top left corner
    int m_rows_top = 0;
    int m_row_height = 0;
    int m_label_x = 0;
    int m_value_x = 0;
    Rect m_progress{};  // the bar: palette::surface track, palette::accent for what has arrived
    Point m_state{};    // top left corner
    Point m_hint{};     // top right corner
};

}  // namespace avela::skin_piccolo

#if CONFIG_AVELA_SCREEN_LANDSCAPE_320X170
#include "avela_skin_piccolo/layouts/landscape_320x170/firmware_update.hpp"
#endif
