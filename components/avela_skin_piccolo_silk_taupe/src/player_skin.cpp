#include "avela_skin_piccolo_silk_taupe/player_skin.hpp"

#include <algorithm>

#include "avela_skin_piccolo_silk_taupe/palette.hpp"
#include "avela_strings/fixed_string.hpp"

namespace avela::skin_piccolo_silk_taupe {
namespace {

//--------------------------------------------------------------------------------------------------

// Bar colours run from spectrum_low over spectrum_middle to spectrum_high, left to right.
uint32_t spectrum_color(int bar, int count)
{
    const auto position = static_cast<uint32_t>(bar) * 2 * full_coverage / static_cast<uint32_t>(count - 1);
    return position < full_coverage ? blend(palette::spectrum_low, palette::spectrum_middle, position)
                                    : blend(palette::spectrum_middle, palette::spectrum_high, position - full_coverage);
}

}  // namespace

//--------------------------------------------------------------------------------------------------

void PlayerSkin::set_station(std::string_view name, std::string_view genre)
{
    bool changed = keep_text(m_name_, name);
    changed = keep_text(m_genre_, genre) || changed;
    if (changed) {
        render(layout::player.m_text_area, &PlayerSkin::draw_text);
    }
}

//--------------------------------------------------------------------------------------------------

void PlayerSkin::set_thumbnail(const Thumbnail& thumbnail)
{
    if (keep_text(m_monogram_, thumbnail.m_monogram)) {
        render(layout::player.m_station_tile.m_area, &PlayerSkin::draw_thumbnail);
    }
}

//--------------------------------------------------------------------------------------------------

void PlayerSkin::set_stream(std::string_view title, std::string_view format)
{
    bool changed = keep_text(m_title_, title);
    changed = keep_text(m_format_, format) || changed;
    if (changed) {
        render(layout::player.m_text_area, &PlayerSkin::draw_text);
    }
}

//--------------------------------------------------------------------------------------------------

void PlayerSkin::set_playing(bool playing)
{
    if (playing != m_playing_) {
        m_playing_ = playing;
        render(layout::player.m_text_area, &PlayerSkin::draw_text);  // the line under the station name
        render(layout::player.m_spectrum.m_area, &PlayerSkin::draw_spectrum);
    }
}

//--------------------------------------------------------------------------------------------------

void PlayerSkin::set_playback_state(PlaybackState state)
{
    if (state != m_playback_state_) {
        m_playback_state_ = state;
        render(layout::player.m_text_area, &PlayerSkin::draw_text);
    }
}

//--------------------------------------------------------------------------------------------------

// Each bar shows a band (spread over the bars when the counts differ) and moves a step towards its level; its peak
// rises with it, holds, then sinks.
void PlayerSkin::set_spectrum(std::span<const uint8_t> spectrum)
{
    bool changed = false;
    for (std::size_t bar = 0; bar < bar_count; ++bar) {
        const int level = spectrum.empty() ? 0 : spectrum[bar * spectrum.size() / bar_count];
        const int shown = m_bars_[bar];
        const int distance = level - shown;
        int step = distance * (distance > 0 ? rise_sixteenths : fall_sixteenths) / sixteenths;
        if (step == 0 && distance != 0) {
            step = distance > 0 ? 1 : -1;  // the last bit of the way
        }
        const int bar_level = shown + step;
        int peak = m_peaks_[bar];
        if (bar_level >= peak) {
            peak = bar_level;
            m_peak_holds_[bar] = peak_hold_frames;
        } else if (m_peak_holds_[bar] > 0) {
            --m_peak_holds_[bar];
        } else {
            peak = std::max(bar_level, peak - peak_fall_levels);
        }
        if (bar_level != shown || peak != m_peaks_[bar]) {
            m_bars_[bar] = static_cast<uint8_t>(bar_level);
            m_peaks_[bar] = static_cast<uint8_t>(peak);
            changed = true;
        }
    }
    if (changed) {
        render(layout::player.m_spectrum.m_area, &PlayerSkin::draw_spectrum);
    }
}

//--------------------------------------------------------------------------------------------------

void PlayerSkin::draw()
{
    m_shown_ = true;
    m_surface_.m_canvas.render(layout::content, palette::background, [&](Painter& painter) {
        draw_thumbnail(painter);
        draw_text(painter);
        draw_spectrum(painter);
    });
}

//--------------------------------------------------------------------------------------------------

void PlayerSkin::render(Rect area, void (PlayerSkin::*draw_part)(Painter&) const) const
{
    if (m_shown_) {
        m_surface_.m_canvas.render(area, palette::background, [&](Painter& painter) { (this->*draw_part)(painter); });
    }
}

//--------------------------------------------------------------------------------------------------

void PlayerSkin::draw_thumbnail(Painter& painter) const
{
    constexpr const auto& tile = layout::player.m_station_tile;
    constexpr Point center{.m_x = tile.m_area.m_x + tile.m_area.m_width / 2,
                           .m_y = tile.m_area.m_y + tile.m_area.m_height / 2};
    painter.fill_round_rect(tile.m_area, tile.m_corner_radius, palette::surface);
    painter.ring(static_cast<float>(center.m_x), static_cast<float>(center.m_y), tile.m_ring_radius,
                 tile.m_ring_thickness, 0.0f, full_turn_degrees, palette::text, palette::surface);
    painter.text(m_monogram_.c_str(), center.m_x, center.m_y + tile.m_monogram_drop, m_surface_.m_fonts.m_heading.get(),
                 palette::text, lgfx::textdatum_t::middle_center);
}

//--------------------------------------------------------------------------------------------------

void PlayerSkin::draw_text(Painter& painter) const
{
    constexpr const auto& player = layout::player;
    const Fonts& fonts = m_surface_.m_fonts;
    painter.text(m_format_.c_str(), player.m_stream_format.m_x, player.m_stream_format.m_y, fonts.m_caption.get(),
                 palette::text_secondary, lgfx::textdatum_t::middle_left);

    painter.clip(player.m_text_clip);
    painter.text(m_name_.c_str(), player.m_station_name.m_x, player.m_station_name.m_y, fonts.m_title.get(),
                 palette::text);
    painter.text(stream_line(), player.m_stream_title.m_x, player.m_stream_title.m_y, fonts.m_subtitle.get(),
                 m_playback_state_ == PlaybackState::playing ? palette::text : palette::text_secondary);
    painter.text(m_genre_.c_str(), player.m_genre.m_x, player.m_genre.m_y, fonts.m_caption.get(),
                 palette::text_secondary);
    painter.clear_clip();
}

//--------------------------------------------------------------------------------------------------

const char* PlayerSkin::stream_line() const
{
    switch (m_playback_state_) {
    case PlaybackState::stopped:
        return m_playing_ ? "Waiting for Wi-Fi" : "Stopped";
    case PlaybackState::connecting:
        return "Connecting…";
    case PlaybackState::failed:
        return "The station doesn't answer";
    case PlaybackState::playing:
        break;
    }
    return m_title_.c_str();
}

//--------------------------------------------------------------------------------------------------

void PlayerSkin::draw_spectrum(Painter& painter) const
{
    constexpr const auto& spectrum = layout::player.m_spectrum;
    constexpr int count = spectrum.m_bar_count;
    constexpr int spread =
        spectrum.m_right - spectrum.m_left - spectrum.m_bar_width;  // first bar's left -> last bar's left
    constexpr int level_max = RadioModel::spectrum_level_max;
    constexpr int rise = spectrum.m_bar_max_height - spectrum.m_bar_min_height;
    for (int bar = 0; bar < count; ++bar) {
        const auto index = static_cast<std::size_t>(bar);
        const int x = spectrum.m_left + (bar * spread + (count - 1) / 2) / (count - 1);
        if (!m_playing_) {  // stopped: a quiet baseline
            painter.fill_rect(Rect{.m_x = x,
                                   .m_y = spectrum.m_bottom - spectrum.m_bar_min_height,
                                   .m_width = spectrum.m_bar_width,
                                   .m_height = spectrum.m_bar_min_height},
                              palette::surface);
            continue;
        }
        const uint32_t color = spectrum_color(bar, count);
        const int height = spectrum.m_bar_min_height + ((m_bars_[index] * rise) + (level_max / 2)) / level_max;
        painter.fill_round_rect(
            Rect{.m_x = x, .m_y = spectrum.m_bottom - height, .m_width = spectrum.m_bar_width, .m_height = height},
            std::min(spectrum.m_bar_corner_radius, height / 2), color);
        if (m_peaks_[index] > 0) {  // the peak's cap, over the bar
            const int peak_height =
                spectrum.m_bar_min_height + ((m_peaks_[index] * rise) + (level_max / 2)) / level_max;
            painter.fill_rect(
                Rect{.m_x = x,
                     .m_y = spectrum.m_bottom - peak_height - spectrum.m_peak_gap - spectrum.m_peak_height,
                     .m_width = spectrum.m_bar_width,
                     .m_height = spectrum.m_peak_height},
                color);
        }
    }
}

}  // namespace avela::skin_piccolo_silk_taupe
