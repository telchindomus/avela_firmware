// Shared layout for the 320x170 landscape screen (CYD 1.9").
// Included by header_layout.hpp only, after the layout structs.
#pragma once

namespace avela::skin_piccolo::layout {

inline constexpr HeaderLayout header{
    .m_area = {.m_x = 0, .m_y = 0, .m_width = screen.m_width, .m_height = 28},
    .m_divider_y = 27,
    .m_logo = {.m_x = 10, .m_y = 4},
    .m_brand_gap = 7,
    .m_text_center_y = 14,
    .m_tray_right = 310,
};

// Everything below the header; the screens' skins draw here.
inline constexpr Rect content{.m_x = 0,
                              .m_y = header.m_area.bottom(),
                              .m_width = screen.m_width,
                              .m_height = screen.m_height - header.m_area.bottom()};

inline constexpr int footer_divider_y = 134;  // line above the bottom band (hints)

}  // namespace avela::skin_piccolo::layout
