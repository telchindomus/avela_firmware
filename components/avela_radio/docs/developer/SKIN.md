# `avela_radio` for skin authors

The radio supplies a player page and a Stations page. The shared header shows requested playback and volume on both;
these pages do not own the header. [Radio behavior](avela_radio.md) and
[application controls](../../../../docs/developer/FEATURES.md#commands-on-each-screen) are defined separately from
presentation.

## Player: `Skin::PlayerSkin` (fulfils `PlayerSkinConcept`)

Construct as `Skin::PlayerSkin{surface}`. The view provides current station identity, reception/format, requested
play state, actual playback state, and spectrum.

| Setter | Values | Meaning |
|---|---|---|
| `set_station(name, genre)` | Text, text | Selected station |
| `set_thumbnail(const Thumbnail&)` | `m_monogram` text | Draw the monogram supplied for the station |
| `set_stream(title, format)` | Text, text; for example `128 kbit/s MP3` | Title may be empty even while playing; values are empty until reported |
| `set_playing(bool)` | Requested playing state | Distinct from actual audio activity |
| `set_playback_state(PlaybackState)` | stopped, connecting, playing, failed | Explain a nonplaying state where stream information would appear |
| `set_spectrum(std::span<const uint8_t>)` | 32 bands, low to high, values 0–255 | Skin chooses bar count/appearance and motion |
| `draw()` | — | Entire content area |

When playback is requested but the state is stopped, the home network is not yet available. Official skins use
messages such as `Connecting…`, `Waiting for Wi-Fi`, and `The station doesn't answer` for nonplaying states. The
actual state and requested play flag must remain distinguishable.

Value limits come from `RadioModel`. Concepts are in `include/avela_radio/ui/player_skin_concept.hpp`. The official
player_skin implementations show spectrum rendering, including peaks.

## `Station` list: `Skin::StationsSkin` (fulfils `StationsSkinConcept`)

Construct as `Skin::StationsSkin{surface}`.

| Setter | Values | Meaning |
|---|---|---|
| `set_selection(stations, selected, playing)` | `std::span<const Station>`, two indexes | Stations with `m_name`, `m_monogram`, `m_genre`; selected and playing indexes |
| `draw()` | — | Entire content area |

Choose how many rows to show around selection. `Station` text belongs to `StationList` and must not outlive its
owning model. The [radio limitations](avela_radio.md#limitations-and-tests) describe the official skins' refresh
behavior after remote edits.

The concept is `include/avela_radio/ui/stations_skin_concept.hpp`. The official stations_skin implementations
provide reference layouts. The shared
[drawing lifecycle](../../../avela_ui/docs/developer/SKIN.md#drawing-rules-for-every-skin) applies to both pages.
Hints should follow the linked application control table rather than implement input handling.
