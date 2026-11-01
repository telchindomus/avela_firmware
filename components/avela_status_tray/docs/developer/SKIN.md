# `avela_status_tray` for skin authors: header

`StatusTrayView` passes clock, playback, volume, and Wi-Fi values to the header skin above every screen. Main owns
the view. Playback input remains in screen controllers; the tray has no controller.

## `Skin::HeaderSkin` (fulfils `StatusTraySkinConcept`)

Construct the skin with the shared surface. It draws the full header band, including the logo/brand area, status
indicators, and separation from the content.

| Setter | Values | Meaning |
|---|---|---|
| `set_time(const LocalTime&)` | `m_year`, `m_month` 1–12, `m_day`, `m_weekday` 0=Sunday, `m_hour`, `m_minute`, `m_synchronised` | Show `--:--` and no date until synchronized |
| `set_playback(bool playing, PlaybackState)` | Requested play state; stopped, connecting, playing, failed | Show audio activity only while actual state is playing |
| `set_volume(int)` | `RadioModel::volume_min`..volume_max, currently 0–100 | Requested volume |
| `set_wifi(WifiStatus, int bars)` | not_configured, connecting, connected; 0–3 bars | Signal bars are meaningful only while connected |
| `draw()` | — | Draw the entire band |

Every setter is called each application frame. The clock's minute changes once per minute, but a skin can use the
same calls to advance motion. Use `std::chrono::steady_clock` in the setter, compute the visible step, and redraw
only when it changes. Follow the
[shared drawing lifecycle](../../../avela_ui/docs/developer/SKIN.md#drawing-rules-for-every-skin).

## Reference appearance

The official piccolo headers show logo/brand, clock/date, Wi-Fi arcs, six volume bars lit in order, and LIVE. The
LIVE pill has an antenna while playback is requested, outgoing waves while audio is playing, and an empty frame when
stopped. Wi-Fi arcs advance while joining. These are implemented by the official header_skin files; exact placement
and motion belong to the skin.

Playback input availability is defined in
[Controls](../../../../docs/developer/FEATURES.md#commands-on-each-screen). Required interface:
`include/avela_status_tray/status_tray_skin_concept.hpp`.
