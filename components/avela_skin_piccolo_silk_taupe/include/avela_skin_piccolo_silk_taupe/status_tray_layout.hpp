// Layout of the status tray in the Silk Taupe skin's header. Values per screen in layouts/<screen>/status_tray.hpp.
// Positions are relative to the tray's right edge and centre line.
#pragma once

#include <array>
#include <cstddef>

#include "avela_skin_piccolo_silk_taupe/screen.hpp"

namespace avela::skin_piccolo_silk_taupe {

// Clock, date and icons from right to left: Wi-Fi, volume, LIVE.
struct TrayLayout {
    // LIVE: a pill with the word while play is wanted, an empty frame while stopped, as tall as the tallest volume bar;
    // an antenna (a short stroke) stands on the middle of its top edge, and while sound comes out waves go out from
    // its tip to both sides, one after another: ((|)).
    struct LiveIcon {
        int m_width = 0;
        int m_height = 0;
        int m_bottom_offset = 0;    // its bottom edge below the tray's centre line
        int m_frame_thickness = 0;  // the empty frame's line, while stopped
        int m_antenna_width = 0;
        int m_antenna_height = 0;             // above the top edge
        std::array<float, 2> m_wave_radii{};  // from the smallest; centred on the antenna's tip
        float m_wave_thickness = 0.0f;
        float m_wave_sweep_degrees = 0.0f;  // each wave, centred on the horizontal
    };

    // Volume: bars rising left to right, lit row by row from the bottom of the first bar up to the volume.
    struct VolumeIcon {
        static constexpr std::size_t bar_count = 6;

        std::array<int, bar_count> m_bar_heights{};
        int m_bar_width = 0;
        int m_bar_gap = 0;
        int m_bottom_offset = 0;  // the bars' bottom below the tray's centre line
    };

    // Wi-Fi icon: three arcs over a dot.
    struct WifiIcon {
        int m_width = 0;          // the icon's room in the tray; its centre sits in the middle
        int m_bottom_offset = 0;  // centre of the arcs below the tray's centre line
        std::array<float, 3> m_arc_radii{};
        float m_arc_thickness = 0.0f;
        float m_arc_start_degrees = 0.0f;
        float m_arc_sweep_degrees = 0.0f;
        int m_dot_radius = 0;
        int m_dot_lift = 0;  // dot centre above the arcs' centre
    };

    int m_date_gap = 0;  // date's right edge -> clock
    int m_icon_gap = 0;  // icon's right edge -> date (or clock while there is no date)
    WifiIcon m_wifi_icon{};
    VolumeIcon m_volume_icon{};
    LiveIcon m_live_icon{};
};

}  // namespace avela::skin_piccolo_silk_taupe

#if CONFIG_AVELA_SCREEN_LANDSCAPE_320X170
#include "avela_skin_piccolo_silk_taupe/layouts/landscape_320x170/status_tray.hpp"
#endif
