#include "avela_skin_piccolo/widgets.hpp"

#include "avela_skin_piccolo/header_layout.hpp"
#include "avela_skin_piccolo/palette.hpp"
#include "avela_skin_piccolo/screen.hpp"

namespace avela::skin_piccolo {

//--------------------------------------------------------------------------------------------------

void draw_badge(Painter& painter, Rect area, const char* text, const Fonts& fonts)
{
    painter.fill_round_rect(area, area.m_height / 2, palette::accent);
    painter.text(text, area.m_x + area.m_width / 2, area.m_y + area.m_height / 2, fonts.m_badge.get(),
                 palette::on_accent, lgfx::textdatum_t::middle_center);
}

//--------------------------------------------------------------------------------------------------

void draw_footer_divider(Painter& painter)
{
    painter.horizontal_line(0, layout::footer_divider_y, screen.m_width, palette::divider);
}

}  // namespace avela::skin_piccolo
