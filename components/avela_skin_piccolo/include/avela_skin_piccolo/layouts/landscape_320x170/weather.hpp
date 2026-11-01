// The weather screen on the 320x170 landscape screen (CYD 1.9").
// Included by weather_layout.hpp only, after the layout structs.
#pragma once

namespace avela::skin_piccolo::layout {

inline constexpr WeatherLayout weather{
    .m_icon_center = {.m_x = 32, .m_y = 62},
    .m_icon_size = 44,
    .m_temperature = {.m_x = 62, .m_y = 36},
    .m_condition = {.m_x = 64, .m_y = 88},
    .m_details = {.m_x = 64, .m_y = 108},
    .m_column_x = 190,
    .m_high_y = 50,
    .m_low_y = 66,
    .m_place_y = 90,
    .m_column_clip_right = 254,
    .m_ring_center = {.m_x = 284, .m_y = 66},
    .m_ring_radius = 19.0f,
    .m_ring_thickness = 3.0f,
    .m_ring_caption = {.m_x = 284, .m_y = 100},
    .m_forecast = {.m_left = 4,
                   .m_cell_width = 78,
                   .m_center_y = 152,
                   .m_weekday_x = 6,
                   .m_icon_x = 40,
                   .m_icon_size = 16,
                   .m_temperature_right = 74},
    .m_message = {.m_x = 16, .m_y = 66},
    .m_message_hint = {.m_x = 16, .m_y = 94},
};

}  // namespace avela::skin_piccolo::layout
