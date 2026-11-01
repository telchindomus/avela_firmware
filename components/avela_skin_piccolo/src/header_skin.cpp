#include "avela_skin_piccolo/header_skin.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <numeric>
#include <span>

#include "avela_radio/model/radio_model.hpp"
#include "avela_skin_piccolo/assets.hpp"
#include "avela_skin_piccolo/header_layout.hpp"
#include "avela_skin_piccolo/palette.hpp"
#include "avela_skin_piccolo/status_tray_layout.hpp"
#include "avela_skin_piccolo/widgets.hpp"
#include "avela_strings/fixed_string.hpp"

namespace avela::skin_piccolo {
namespace {

//--------------------------------------------------------------------------------------------------

const char* weekday_name(int weekday)
{
    static constexpr std::array<const char*, 7> names{"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    return names[static_cast<std::size_t>(weekday) % names.size()];
}

//--------------------------------------------------------------------------------------------------

const char* month_name(int month)
{
    static constexpr std::array<const char*, 12> names{"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                                       "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    return names[static_cast<std::size_t>(month - 1) % names.size()];
}

//--------------------------------------------------------------------------------------------------

// While joining, the arcs light up one after another (none, 1, 2, 3, again), a step every joining_step_duration.
// No task: the tray calls set_wifi() every frame and the step follows the steady clock.
constexpr auto joining_step_duration = std::chrono::milliseconds{300};
constexpr int joining_steps = 4;

int joining_step()
{
    const auto elapsed = std::chrono::steady_clock::now().time_since_epoch();
    return static_cast<int>((elapsed / joining_step_duration) % joining_steps);
}

//--------------------------------------------------------------------------------------------------

// While sound comes out, waves go out from LIVE's antenna one after another (none, 1, 2, again), a step every
// emitting_step_duration; like the joining, the step follows the steady clock.
constexpr auto emitting_step_duration = std::chrono::milliseconds{400};
constexpr int emitting_steps = 3;

int emitting_step()
{
    const auto elapsed = std::chrono::steady_clock::now().time_since_epoch();
    return static_cast<int>((elapsed / emitting_step_duration) % emitting_steps);
}

//--------------------------------------------------------------------------------------------------

constexpr int volume_icon_width = [] {
    constexpr const auto& icon = layout::tray.m_volume_icon;
    const auto bar_count = static_cast<int>(icon.m_bar_heights.size());
    return (bar_count * icon.m_bar_width) + ((bar_count - 1) * icon.m_bar_gap);
}();

// The volume as bars rising left to right from `left`, their bottom on `bottom_y`, lit row by row: the volume's share
// of all the bars' rows, from the bottom of the first bar up (the quietest volume lights one row, silence none).
void draw_volume_icon(Painter& painter, int left, int bottom_y, int volume)
{
    constexpr const auto& icon = layout::tray.m_volume_icon;
    constexpr int rows = std::accumulate(icon.m_bar_heights.begin(), icon.m_bar_heights.end(), 0);
    constexpr int range = RadioModel::volume_max - RadioModel::volume_min;
    int lit_rows = (((volume - RadioModel::volume_min) * rows) + range - 1) / range;
    int x = left;
    for (const int height : icon.m_bar_heights) {
        const int lit = std::clamp(lit_rows, 0, height);
        painter.fill_rect(
            Rect{.m_x = x, .m_y = bottom_y - height, .m_width = icon.m_bar_width, .m_height = height - lit},
            palette::volume_track);
        painter.fill_rect(Rect{.m_x = x, .m_y = bottom_y - lit, .m_width = icon.m_bar_width, .m_height = lit},
                          palette::volume_lit);
        lit_rows -= lit;
        x += icon.m_bar_width + icon.m_bar_gap;
    }
}

//--------------------------------------------------------------------------------------------------

// LIVE with its right edge on `right` and its bottom on `bottom_y`: the pill and a lit antenna while play is wanted,
// with `waves` waves on both sides of the antenna's tip; the empty frame and an unlit antenna while stopped.
void draw_live_icon(Painter& painter, int right, int bottom_y, bool playing, int waves, const Fonts& fonts)
{
    constexpr const auto& icon = layout::tray.m_live_icon;
    const Rect area{.m_x = right - icon.m_width,
                    .m_y = bottom_y - icon.m_height,
                    .m_width = icon.m_width,
                    .m_height = icon.m_height};
    const int radius = icon.m_height / 2;
    const int center_x = area.m_x + (area.m_width / 2);
    painter.fill_rect(Rect{.m_x = center_x - (icon.m_antenna_width / 2),
                           .m_y = area.m_y - icon.m_antenna_height,
                           .m_width = icon.m_antenna_width,
                           .m_height = icon.m_antenna_height},
                      playing ? palette::volume_lit : palette::live_frame);
    if (!playing) {
        constexpr int frame = icon.m_frame_thickness;
        painter.fill_round_rect(area, radius, palette::live_frame);
        painter.fill_round_rect(Rect{.m_x = area.m_x + frame,
                                     .m_y = area.m_y + frame,
                                     .m_width = area.m_width - (2 * frame),
                                     .m_height = area.m_height - (2 * frame)},
                                radius - frame, palette::background);
        return;
    }
    draw_badge(painter, area, "LIVE", fonts);
    // a wave to the left is centred on 180 degrees, one to the right on 0
    const auto tip_x = static_cast<float>(center_x);
    const auto tip_y = static_cast<float>(area.m_y - icon.m_antenna_height);
    constexpr float half_sweep = icon.m_wave_sweep_degrees / 2.0f;
    constexpr float left_degrees = (full_turn_degrees / 2.0f) - half_sweep;
    constexpr float right_degrees = full_turn_degrees - half_sweep;
    for (const float wave_radius : std::span(icon.m_wave_radii).first(static_cast<std::size_t>(waves))) {
        painter.ring(tip_x, tip_y, wave_radius, icon.m_wave_thickness, left_degrees, icon.m_wave_sweep_degrees,
                     palette::volume_lit, palette::background);
        painter.ring(tip_x, tip_y, wave_radius, icon.m_wave_thickness, right_degrees, icon.m_wave_sweep_degrees,
                     palette::volume_lit, palette::background);
    }
}

//--------------------------------------------------------------------------------------------------

// `lit_arcs` arcs lit from the smallest (the largest arc is the third); the dot shows whether a network is saved.
void draw_wifi_icon(Painter& painter, int center_x, int bottom_y, WifiStatus status, int lit_arcs)
{
    constexpr const auto& icon = layout::tray.m_wifi_icon;
    int arc = static_cast<int>(icon.m_arc_radii.size());  // arc_radii run from the largest arc down
    for (const float radius : icon.m_arc_radii) {
        const uint32_t color = arc <= lit_arcs ? palette::wifi_lit : palette::wifi_unlit;
        painter.ring(static_cast<float>(center_x), static_cast<float>(bottom_y), radius, icon.m_arc_thickness,
                     icon.m_arc_start_degrees, icon.m_arc_sweep_degrees, color, palette::background);
        --arc;
    }
    const uint32_t dot = status == WifiStatus::not_configured ? palette::wifi_dot_off : palette::wifi_dot_on;
    painter.fill_circle(center_x, bottom_y - icon.m_dot_lift, icon.m_dot_radius, dot);
}

}  // namespace

//--------------------------------------------------------------------------------------------------

void HeaderSkin::set_time(const LocalTime& time)
{
    // the tray shows minutes (and the date with them), so only a new minute or the clock being set redraws it
    const bool changed = time.m_synchronised != m_time_.m_synchronised ||
                         (time.m_synchronised && (time.m_minute != m_time_.m_minute || time.m_hour != m_time_.m_hour ||
                                                  time.m_day != m_time_.m_day));
    m_time_ = time;
    if (changed) {
        render();
    }
}

//--------------------------------------------------------------------------------------------------

void HeaderSkin::set_playback(bool playing, PlaybackState state)
{
    const int waves = playing && state == PlaybackState::playing ? emitting_step() : 0;
    if (playing == m_playing_ && waves == m_waves_) {
        return;
    }
    m_playing_ = playing;
    m_waves_ = waves;
    render();
}

//--------------------------------------------------------------------------------------------------

void HeaderSkin::set_volume(int volume)
{
    if (volume != m_volume_) {
        m_volume_ = volume;
        render();
    }
}

//--------------------------------------------------------------------------------------------------

void HeaderSkin::set_wifi(WifiStatus status, int bars)
{
    int lit_arcs = 0;
    if (status == WifiStatus::connected) {
        lit_arcs = bars;
    } else if (status == WifiStatus::connecting) {
        lit_arcs = joining_step();
    }
    if (status == m_wifi_status_ && lit_arcs == m_lit_arcs_) {
        return;
    }
    m_wifi_status_ = status;
    m_lit_arcs_ = lit_arcs;
    render();
}

//--------------------------------------------------------------------------------------------------

void HeaderSkin::draw()
{
    m_shown_ = true;
    render();
}

//--------------------------------------------------------------------------------------------------

void HeaderSkin::render() const
{
    if (m_shown_) {
        m_surface_.m_canvas.render(layout::header.m_area, palette::background, [&](Painter& painter) {
            draw_brand(painter);
            draw_tray(painter);
        });
    }
}

//--------------------------------------------------------------------------------------------------

// Logo, "Avela" and the divider line.
void HeaderSkin::draw_brand(Painter& painter) const
{
    constexpr const auto& header = layout::header;
    painter.image_with_alpha(header.m_logo.m_x, header.m_logo.m_y, assets::logo_width, assets::logo_height,
                             assets::avela_logo_argb().data());
    painter.text("Avela", header.m_logo.m_x + assets::logo_width + header.m_brand_gap, header.m_text_center_y,
                 m_surface_.m_fonts.m_heading.get(), palette::text, lgfx::textdatum_t::middle_left);
    painter.horizontal_line(0, header.m_divider_y, screen.m_width, palette::divider);
}

//--------------------------------------------------------------------------------------------------

// Right to left from the tray's right edge: clock, date, Wi-Fi, volume, LIVE.
void HeaderSkin::draw_tray(Painter& painter) const
{
    const Fonts& fonts = m_surface_.m_fonts;
    const Point right_center{.m_x = layout::header.m_tray_right, .m_y = layout::header.m_text_center_y};

    FixedString<capacity_for("23:59")> clock_text;
    if (m_time_.m_synchronised) {
        clock_text.append_number(m_time_.m_hour, 2).append(":").append_number(m_time_.m_minute, 2);
    } else {
        clock_text.append("--:--");
    }
    painter.text(clock_text.c_str(), right_center.m_x, right_center.m_y, fonts.m_figure.get(), palette::text,
                 lgfx::textdatum_t::middle_right);
    int left = right_center.m_x - painter.text_width(clock_text.c_str(), fonts.m_figure.get());

    if (m_time_.m_synchronised) {
        FixedString<capacity_for("Wed, 30 Sep")> date;  // weekday and month names have 3 letters
        date.append(weekday_name(m_time_.m_weekday))
            .append(", ")
            .append_number(m_time_.m_day)
            .append(" ")
            .append(month_name(m_time_.m_month));
        const int date_right = left - layout::tray.m_date_gap;
        painter.text(date.c_str(), date_right, right_center.m_y, fonts.m_body.get(), palette::text_secondary,
                     lgfx::textdatum_t::middle_right);
        left = date_right - painter.text_width(date.c_str(), fonts.m_body.get());
    }

    const int icon_right = left - layout::tray.m_icon_gap;
    draw_wifi_icon(painter, icon_right - layout::tray.m_wifi_icon.m_width / 2,
                   right_center.m_y + layout::tray.m_wifi_icon.m_bottom_offset, m_wifi_status_, m_lit_arcs_);

    const int volume_right = icon_right - layout::tray.m_wifi_icon.m_width - layout::tray.m_icon_gap;
    draw_volume_icon(painter, volume_right - volume_icon_width,
                     right_center.m_y + layout::tray.m_volume_icon.m_bottom_offset, m_volume_);
    draw_live_icon(painter, volume_right - volume_icon_width - layout::tray.m_icon_gap,
                   right_center.m_y + layout::tray.m_live_icon.m_bottom_offset, m_playing_, m_waves_, fonts);
}

}  // namespace avela::skin_piccolo
