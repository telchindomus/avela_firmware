#include "avela_skin_piccolo_silk_taupe/stations_skin.hpp"

#include <initializer_list>

#include "avela_skin_piccolo_silk_taupe/palette.hpp"
#include "avela_skin_piccolo_silk_taupe/station_list_layout.hpp"
#include "avela_skin_piccolo_silk_taupe/widgets.hpp"
#include "avela_strings/fixed_string.hpp"

namespace avela::skin_piccolo_silk_taupe {

//--------------------------------------------------------------------------------------------------

void StationsSkin::set_selection(std::span<const Station> stations, std::size_t selected, std::size_t playing)
{
    if (stations.data() == m_stations_.data() && stations.size() == m_stations_.size() && selected == m_selected_ &&
        playing == m_playing_) {
        return;
    }
    m_stations_ = stations;
    m_selected_ = selected;
    m_playing_ = playing;
    if (m_shown_) {
        m_surface_.m_canvas.render(layout::station_list.m_area, palette::background,
                                   [&](Painter& painter) { draw_list(painter); });
    }
}

//--------------------------------------------------------------------------------------------------

void StationsSkin::draw()
{
    m_shown_ = true;
    m_surface_.m_canvas.render(layout::content, palette::background, [&](Painter& painter) {
        draw_list(painter);
        draw_footer_divider(painter);
        painter.text("Turn: choose · Press: play", layout::station_list.m_hint.m_x, layout::station_list.m_hint.m_y,
                     m_surface_.m_fonts.m_caption.get(), palette::label, lgfx::textdatum_t::middle_center);
    });
}

//--------------------------------------------------------------------------------------------------

std::size_t StationsSkin::wrapped(int offset) const
{
    const int count = static_cast<int>(m_stations_.size());
    return static_cast<std::size_t>(((static_cast<int>(m_selected_) + offset) % count + count) % count);
}

//--------------------------------------------------------------------------------------------------

void StationsSkin::draw_list(Painter& painter) const
{
    constexpr const auto& list = layout::station_list;
    const Fonts& fonts = m_surface_.m_fonts;

    // neighbours, dim
    for (const int offset : {-1, +1}) {
        const Station& station = m_stations_[wrapped(offset)];
        const int center_y = offset < 0 ? list.m_previous_center_y : list.m_next_center_y;
        painter.text(station.m_name, list.m_neighbour_text_x, center_y, fonts.m_list_item.get(),
                     palette::text_secondary, lgfx::textdatum_t::middle_left);
        if (wrapped(offset) == m_playing_) {
            draw_playing_marker(painter, list.m_current_marker_x, center_y);
        }
    }

    // selected row
    constexpr Rect row = list.m_selected_row;
    painter.fill_round_rect(row, list.m_row_corner_radius, palette::surface);
    painter.fill_round_rect(Rect{.m_x = row.m_x,
                                 .m_y = row.m_y + list.m_accent_inset,
                                 .m_width = list.m_accent_width,
                                 .m_height = row.m_height - 2 * list.m_accent_inset},
                            list.m_accent_corner_radius, palette::accent);
    const Station& station = m_stations_[m_selected_];
    painter.clip(Rect{.m_x = row.m_x + list.m_text_clip_inset,
                      .m_y = row.m_y,
                      .m_width = list.m_text_clip_width,
                      .m_height = row.m_height});
    painter.text(station.m_name, row.m_x + list.m_name_offset.m_x, row.m_y + list.m_name_offset.m_y,
                 fonts.m_heading.get(), palette::text);
    painter.text(station.m_genre, row.m_x + list.m_genre_offset.m_x, row.m_y + list.m_genre_offset.m_y,
                 fonts.m_caption.get(), palette::text_secondary);
    painter.clear_clip();
    FixedString<capacity_for("999 / 999")> position;  // shown with two digits; room for 999 stations
    position.append_number(m_selected_ + 1, 2).append(" / ").append_number(m_stations_.size(), 2);
    const int position_right = row.right() - list.m_position_right_inset;
    const int row_center_y = row.m_y + row.m_height / 2;
    painter.text(position.c_str(), position_right, row_center_y, fonts.m_caption.get(), palette::label,
                 lgfx::textdatum_t::middle_right);
    if (m_selected_ == m_playing_) {
        const int position_left = position_right - painter.text_width(position.c_str(), fonts.m_caption.get());
        draw_playing_marker(painter, position_left - list.m_current_marker_gap - list.m_current_marker_radius,
                            row_center_y);
    }
}

//--------------------------------------------------------------------------------------------------

void StationsSkin::draw_playing_marker(Painter& painter, int center_x, int center_y)
{
    const int radius = layout::station_list.m_current_marker_radius;
    painter.fill_triangle(center_x - radius, center_y - radius, center_x - radius, center_y + radius, center_x + radius,
                          center_y, palette::accent);
}

}  // namespace avela::skin_piccolo_silk_taupe
