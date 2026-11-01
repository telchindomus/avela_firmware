// The piccolo skin's player: station tile (the thumbnail), format, station name, stream title (or, until the stream
// plays, what the radio is doing), genre, and below them the spectrum as bars with their peaks (the header shows
// LIVE and the volume on every screen). Each part redraws on its own when its values change; the bars move towards
// the spectrum's levels, rising fast and falling slowly, the peaks hold a moment and then sink. Fulfils
// PlayerSkinConcept (avela_radio).
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

#include "avela_radio/model/radio_model.hpp"
#include "avela_radio/ui/player_skin_concept.hpp"
#include "avela_radio/ui/thumbnail.hpp"
#include "avela_skin_piccolo/player_layout.hpp"
#include "avela_skin_piccolo/surface.hpp"
#include "avela_skin_piccolo/widgets.hpp"

namespace avela::skin_piccolo {

class PlayerSkin {
public:
    explicit PlayerSkin(const Surface& surface) : m_surface_(surface) {}

    void set_station(std::string_view name, std::string_view genre);
    void set_thumbnail(const Thumbnail& thumbnail);
    void set_stream(std::string_view title, std::string_view format);
    void set_playing(bool playing);
    void set_playback_state(PlaybackState state);
    void set_spectrum(std::span<const uint8_t> spectrum);
    void draw();

private:
    // How far a bar moves towards its band's level each frame, in sixteenths of the distance: up fast, down slowly.
    static constexpr int rise_sixteenths = 9;
    static constexpr int fall_sixteenths = 2;
    static constexpr int sixteenths = 16;
    // A peak stays this many frames at its highest level, then sinks by peak_fall_levels a frame.
    static constexpr int peak_hold_frames = 15;
    static constexpr int peak_fall_levels = 4;
    static constexpr auto bar_count = static_cast<std::size_t>(layout::player.m_spectrum.m_bar_count);

    // Redraws one part in its area (the parts' areas don't overlap), once the skin is shown.
    void render(Rect area, void (PlayerSkin::*draw_part)(Painter&) const) const;
    void draw_thumbnail(Painter& painter) const;
    void draw_text(Painter& painter) const;
    void draw_spectrum(Painter& painter) const;
    // The stream's title, or what the radio does instead of playing.
    [[nodiscard]] const char* stream_line() const;

    Surface m_surface_;
    bool m_shown_ = false;
    LabelText m_name_;
    LabelText m_genre_;
    LabelText m_monogram_;
    FixedString<RadioModel::stream_title_capacity> m_title_;
    FixedString<RadioModel::stream_format_capacity> m_format_;
    bool m_playing_ = false;
    PlaybackState m_playback_state_ = PlaybackState::stopped;
    std::array<uint8_t, bar_count> m_bars_{};        // the bars' levels shown
    std::array<uint8_t, bar_count> m_peaks_{};       // the peaks' levels shown
    std::array<uint8_t, bar_count> m_peak_holds_{};  // frames each peak still holds
};

}  // namespace avela::skin_piccolo

namespace avela {
static_assert(PlayerSkinConcept<skin_piccolo::PlayerSkin>);
}  // namespace avela
