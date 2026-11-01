// The Silk Taupe skin's colours by role: mushroom taupe, espresso ink, stone surfaces and champagne gold. Page skins
// use the roles only.
#pragma once

#include <cstdint>

namespace avela::skin_piccolo_silk_taupe::palette {

inline constexpr uint32_t background = 0xB7A69A;  // mushroom taupe
inline constexpr uint32_t divider = 0x948171;
inline constexpr uint32_t surface = 0xC9BBAA;          // stone: tiles, the selected row, empty part of icons
inline constexpr uint32_t text = 0x2C2522;             // espresso: primary text
inline constexpr uint32_t text_secondary = 0x51433A;   // secondary text, icons
inline constexpr uint32_t label = 0x584338;            // labels, hints
inline constexpr uint32_t accent = 0x8C6037;           // champagne gold (bronze base): active elements
inline constexpr uint32_t on_accent = text;            // text on accent (badges)
inline constexpr uint32_t gold_reflection = 0xC5AB79;  // the light in the gold highlights (widgets: draw_gold_glint)
inline constexpr uint32_t volume_track = 0x9D8979;     // the unlit volume bars, quieter than surface

// The QR code: espresso modules on a stone square, about 8:1 contrast (scanners need clearly dark on light).
inline constexpr uint32_t qr_dark = text;
inline constexpr uint32_t qr_light = surface;

// Spectrum bars from the lowest band to the highest, blended in between.
inline constexpr uint32_t spectrum_low = text_secondary;
inline constexpr uint32_t spectrum_middle = accent;
inline constexpr uint32_t spectrum_high = label;

// Weather icons: the sun (and moon), clouds, rain and snow.
inline constexpr uint32_t weather_sun = accent;
inline constexpr uint32_t weather_cloud = text;
inline constexpr uint32_t weather_precipitation = label;

// The tray's icons: lit parts in the accent, unlit parts in the volume track. The Wi-Fi dot in the accent while a
// network is saved (joined or joining), in the track colour without one; LIVE's empty frame (stopped) in the track
// colour.
inline constexpr uint32_t wifi_lit = accent;
inline constexpr uint32_t wifi_unlit = volume_track;
inline constexpr uint32_t wifi_dot_on = accent;
inline constexpr uint32_t wifi_dot_off = volume_track;
inline constexpr uint32_t volume_lit = accent;
inline constexpr uint32_t live_frame = volume_track;

}  // namespace avela::skin_piccolo_silk_taupe::palette
