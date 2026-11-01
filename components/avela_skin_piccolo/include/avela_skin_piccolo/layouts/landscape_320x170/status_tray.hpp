// The status tray on the 320x170 landscape screen (CYD 1.9").
// Included by status_tray_layout.hpp only, after the layout structs.
#pragma once

namespace avela::skin_piccolo::layout {

inline constexpr TrayLayout tray{
    .m_date_gap = 10,
    .m_icon_gap = 7,
    .m_wifi_icon =
        {
            .m_width = 14,
            .m_bottom_offset = 5,
            .m_arc_radii = {10.0f, 7.0f, 4.0f},
            .m_arc_thickness = 1.6f,
            .m_arc_start_degrees = 225.0f,
            .m_arc_sweep_degrees = 90.0f,
            .m_dot_radius = 1,
            .m_dot_lift = 0,
        },
    .m_volume_icon =
        {
            .m_bar_heights = {5, 6, 8, 9, 11, 12},  // 51 rows: every knob step lights or dims at least one
            .m_bar_width = 2,
            .m_bar_gap = 2,
            .m_bottom_offset = 5,
        },
    .m_live_icon =
        {
            .m_width = 26,
            .m_height = 12,  // the tallest volume bar's
            .m_bottom_offset = 5,
            .m_frame_thickness = 1,
            .m_antenna_width = 2,
            .m_antenna_height = 3,
            .m_wave_radii = {3.5f, 6.0f},
            .m_wave_thickness = 1.2f,
            .m_wave_sweep_degrees = 60.0f,
        },
};

}  // namespace avela::skin_piccolo::layout
