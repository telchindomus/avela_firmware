#include "avela_skin_piccolo_silk_taupe/fonts.hpp"

#include "avela_skin_piccolo_silk_taupe/assets.hpp"

namespace avela::skin_piccolo_silk_taupe {

//--------------------------------------------------------------------------------------------------

void Fonts::load()
{
    m_display.load(assets::font_display_vlw());
    m_headline.load(assets::font_headline_vlw());
    m_title.load(assets::font_title_vlw());
    m_heading.load(assets::font_heading_vlw());
    m_figure.load(assets::font_figure_vlw());
    m_subtitle.load(assets::font_subtitle_vlw());
    m_list_item.load(assets::font_list_item_vlw());
    m_body.load(assets::font_body_vlw());
    m_body_strong.load(assets::font_body_strong_vlw());
    m_caption.load(assets::font_caption_vlw());
    m_badge.load(assets::font_badge_vlw());
}

}  // namespace avela::skin_piccolo_silk_taupe
