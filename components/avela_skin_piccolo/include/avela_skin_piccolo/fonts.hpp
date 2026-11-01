// The piccolo skin's fonts by role (Manrope; weight and size per role and screen: SKINS in tools/generate_assets.py).
#pragma once

#include "avela_ui/font.hpp"

namespace avela::skin_piccolo {

struct Fonts {
    Font m_display;      // big numbers: the weather's temperature (digits, minus and degree sign only)
    Font m_headline;     // screen headline
    Font m_title;        // station name on the player
    Font m_heading;      // brand name, selected list row, station monogram
    Font m_figure;       // clock, ring values
    Font m_subtitle;     // stream title
    Font m_list_item;    // unselected list rows
    Font m_body;         // date, labels, previous/next station
    Font m_body_strong;  // values next to labels, volume
    Font m_caption;      // format, genre, hints
    Font m_badge;        // LIVE / INFO pills

    // Once, at startup: the embedded fonts of this build's screen (avela_skin_piccolo/assets.hpp).
    void load();
};

}  // namespace avela::skin_piccolo
