#include "avela_skin_piccolo/tiles_skin.hpp"

#include "avela_skin_piccolo/assets.hpp"
#include "avela_skin_piccolo/palette.hpp"
#include "avela_skin_piccolo/tiles_layout.hpp"
#include "avela_skin_piccolo/widgets.hpp"
#include "sdkconfig.h"
#if CONFIG_AVELA_WEATHER
#include "avela_skin_piccolo/weather_icons.hpp"
#endif

namespace avela::skin_piccolo {
namespace {

constexpr const auto& tiles_layout = layout::tiles;

const char* label_of(ScreenId shown)
{
    switch (shown) {
    case ScreenId::player:
        return "Radio";
    case ScreenId::info:
        return "Info";
#if CONFIG_AVELA_WEATHER
    case ScreenId::weather:
        return "Weather";
#endif
    case ScreenId::tiles:
    case ScreenId::stations:
    case ScreenId::wifi_setup:
    case ScreenId::firmware_update:
        break;  // not on a tile
    }
    return "";
}

//--------------------------------------------------------------------------------------------------

void draw_info_icon(Painter& painter, int center_x, int center_y, uint32_t color, uint32_t background)
{
    constexpr const auto& icon = tiles_layout.m_info_icon;
    painter.ring(static_cast<float>(center_x), static_cast<float>(center_y), icon.m_ring_radius, icon.m_ring_thickness,
                 0.0f, full_turn_degrees, color, background);
    painter.fill_circle(center_x, center_y + icon.m_dot_y, icon.m_dot_radius, color);
    painter.fill_rect(Rect{.m_x = center_x - icon.m_stem_width / 2,
                           .m_y = center_y + icon.m_stem_y,
                           .m_width = icon.m_stem_width,
                           .m_height = icon.m_stem_height},
                      color);
}

}  // namespace

//--------------------------------------------------------------------------------------------------

void TilesSkin::set_tiles(std::span<const ScreenId> screens, std::size_t selected)
{
    const bool changed =
        screens.data() != m_screens_.data() || screens.size() != m_screens_.size() || selected != m_selected_;
    m_screens_ = screens;
    m_selected_ = selected;
    if (changed && m_drawn_) {
        render();
    }
}

//--------------------------------------------------------------------------------------------------

void TilesSkin::draw()
{
    m_drawn_ = true;
    render();
}

//--------------------------------------------------------------------------------------------------

void TilesSkin::render() const
{
    const int count = static_cast<int>(m_screens_.size());
    const int gaps = (count - 1) * tiles_layout.m_gap;
    const int tile_width = count > 0 ? (screen.m_width - (2 * tiles_layout.m_side_margin) - gaps) / count : 0;
    const int left = (screen.m_width - (count * tile_width) - gaps) / 2;

    m_surface_.m_canvas.render(layout::content, palette::background, [&](Painter& painter) {
        for (int index = 0; index < count; ++index) {
            const Rect area{.m_x = left + index * (tile_width + tiles_layout.m_gap),
                            .m_y = tiles_layout.m_top,
                            .m_width = tile_width,
                            .m_height = tiles_layout.m_tile_height};
            const auto tile = static_cast<std::size_t>(index);
            draw_tile(painter, m_screens_[tile], area, tile == m_selected_);
        }
        draw_footer_divider(painter);
        painter.text("Turn: choose · Press: open", tiles_layout.m_hint.m_x, tiles_layout.m_hint.m_y,
                     m_surface_.m_fonts.m_caption.get(), palette::label, lgfx::textdatum_t::middle_center);
    });
}

//--------------------------------------------------------------------------------------------------

void TilesSkin::draw_tile(Painter& painter, ScreenId shown, Rect area, bool selected) const
{
    const Fonts& fonts = m_surface_.m_fonts;
    const int border = selected ? tiles_layout.m_selected_border : tiles_layout.m_border;
    const uint32_t fill = selected ? palette::surface : palette::background;
    // outline: the border colour's rounded rectangle with the fill's inside it
    painter.fill_round_rect(area, tiles_layout.m_corner_radius, selected ? palette::accent : palette::divider);
    painter.fill_round_rect(Rect{.m_x = area.m_x + border,
                                 .m_y = area.m_y + border,
                                 .m_width = area.m_width - 2 * border,
                                 .m_height = area.m_height - 2 * border},
                            tiles_layout.m_corner_radius - border, fill);

    const int center_x = area.m_x + area.m_width / 2;
    const int icon_y = area.m_y + tiles_layout.m_icon_center_y;
    const uint32_t color = selected ? palette::text : palette::text_secondary;
    switch (shown) {
    case ScreenId::player:
        painter.image_with_alpha(center_x - assets::logo_width / 2, icon_y - assets::logo_height / 2,
                                 assets::logo_width, assets::logo_height, assets::avela_logo_argb().data());
        break;
    case ScreenId::info:
        draw_info_icon(painter, center_x, icon_y, color, fill);
        break;
#if CONFIG_AVELA_WEATHER
    case ScreenId::weather:
        draw_weather_icon(painter, Point{.m_x = center_x, .m_y = icon_y}, tiles_layout.m_weather_icon_size,
                          WeatherCondition::partly_cloudy, true, fill);
        break;
#endif
    case ScreenId::tiles:
    case ScreenId::stations:
    case ScreenId::wifi_setup:
    case ScreenId::firmware_update:
        break;  // not on a tile
    }
    painter.text(label_of(shown), center_x, area.m_y + tiles_layout.m_label_center_y,
                 selected ? fonts.m_body_strong.get() : fonts.m_body.get(), color, lgfx::textdatum_t::middle_center);
}

}  // namespace avela::skin_piccolo
