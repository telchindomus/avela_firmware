// The piccolo skin's colours by role (TelchinDomus brandbook palette on the darker Avela teal; the brand's names in the
// comments). Page skins use the roles only.
#pragma once

#include <cstdint>

namespace avela::skin_piccolo::palette {

inline constexpr uint32_t background = 0x0F4248;
inline constexpr uint32_t divider = 0x2A7579;
inline constexpr uint32_t surface = 0x2C6A6C;  // tiles, the selected row, empty part of rings and icons
inline constexpr uint32_t text = 0xF5F2E9;     // Parchment: primary text
// The brand's Seafoam (7DA499) and Lavender (A1A3CB), lightened for small text on the panel
inline constexpr uint32_t text_secondary = 0xA2BDB2;  // Seafoam, lightened: secondary text, icons
inline constexpr uint32_t label = 0xB6B8D9;           // Lavender, lightened: labels, hints
inline constexpr uint32_t accent = 0xD2AF6D;          // Tech Gold: active elements
inline constexpr uint32_t on_accent = background;     // text on accent (badges)
inline constexpr uint32_t volume_track = surface;     // the unlit volume bars

// The QR code: deep teal modules on Parchment, about 11:1 contrast (scanners need clearly dark on light; gold or
// inverted codes fail on many phones).
inline constexpr uint32_t qr_dark = background;
inline constexpr uint32_t qr_light = text;

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

}  // namespace avela::skin_piccolo::palette
