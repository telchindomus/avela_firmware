// Pieces the piccolo skin's skins share: label text, badges, the line above the bottom band, keeping a shown text.
#pragma once

#include <cstddef>
#include <string_view>

#include "avela_skin_piccolo/fonts.hpp"
#include "avela_strings/fixed_string.hpp"
#include "avela_ui/canvas.hpp"

namespace avela::skin_piccolo {

// Free text such as station names or info values. Longer text is cut; the skins clip it earlier anyway.
inline constexpr std::size_t label_capacity = 64;
using LabelText = FixedString<label_capacity>;

// Copies `text` into the text a skin shows; true if that changed it (text that doesn't fit counts as cut).
template <std::size_t Capacity>
bool keep_text(FixedString<Capacity>& shown, std::string_view text)
{
    const std::string_view fitting = text.substr(0, FixedString<Capacity>::max_length());
    if (shown.view() == fitting) {
        return false;
    }
    shown.clear().append(fitting);
    return true;
}

// Accent pill with dark text, e.g. "LIVE".
void draw_badge(Painter& painter, Rect area, const char* text, const Fonts& fonts);

// The line above the bottom band (hints).
void draw_footer_divider(Painter& painter);

}  // namespace avela::skin_piccolo
