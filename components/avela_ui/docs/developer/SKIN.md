# `avela_ui` for skin authors: shared pieces

Every skin supplies the shared pieces below. The UI [developer reference](avela_ui.md) explains controllers and
drawing; the [application reference](../../../../docs/developer/FEATURES.md#controls) owns physical controls and
screen navigation.

## `Skin` (your `<component>/skin.hpp`)

Declare `using Skin = <your struct>;` in namespace avela. The struct names these members:

| Member | Contract |
|---|---|
| `Fonts` | Font collection with `load()`, called once during application initialization |
| `Surface` | Constructed with `m_canvas` and `m_fonts`: `Canvas&` and `const Fonts&` |
| `HeaderSkin` | Shared header implementation; see the [header contract](../../../avela_status_tray/docs/developer/SKIN.md) |
| `TilesSkin` | Screen selector implementation, below |
| Page-skin types | `PlayerSkin`, `StationsSkin`, `InfoSkin`, `WifiSetupSkin`, `FirmwareUpdateSkin`, and optional `WeatherSkin` |
| `static constexpr ScreenSize screen` | Must match the board's post-rotation screen size |

Main owns the fonts and canvas, constructs the surface, and builds each page skin from it, for example
`Skin::PlayerSkin{surface}`. Page skins draw the content area below the header; `HeaderSkin` draws the header band.

## Drawing rules for every skin

Setters take concrete values. Before `draw()`, they store the values to be drawn. `draw()` renders the initial area.
Thereafter, a setter redraws the affected area when visible output changes. Equal data normally draws nothing, but a
changed animation step can require a redraw without changed input data.

Views call setters on each frame, so a skin can use a monotonic clock to animate. Draw only on that calling task; do
not create a drawing task or timer. All callbacks within one band render must draw the same picture. See
[Drawing](avela_ui.md#drawing) for the authoritative band size, buffer dimensions, and coordinate rules. No heap
allocation or virtual functions are used.

## `TilesSkin`: the screen selector

`TilesSkin` fulfils TilesSkinConcept and is built as `Skin::TilesSkin{surface}`. Main provides the enabled screen
list; the component's `TilesController` handles selection and opening screens.

| Setter | Value | Meaning |
|---|---|---|
| `set_tiles(screens, selected)` | `std::span<const ScreenId>`, index | Static-storage list in display order and selected tile |
| `draw()` | — | Draw the whole content area |

Provide a label and icon for every offered screen. Main currently offers player and info, plus weather when enabled,
so the layout handles two or three tiles. The official skins use the Radio logo and primitive-drawn icons for other
tiles. `ScreenId::stations` is also used for Stations-specific icon handling, although it is not a root tile.

When Back returns to tiles, selected identifies the screen just closed. Labels, icons, positions, and selection
appearance belong to the skin. Hints should name physical inputs, for example `Turn: choose · Press: open`, rather
than exposing command type names.

The complete per-screen control mapping is in
[Commands on each screen](../../../../docs/developer/FEATURES.md#commands-on-each-screen); do not implement behavior
in the skin.

## `ScreenId`

`include/avela_ui/screen_id.hpp` defines the enum used by controllers, application screen construction, and tile
drawing. It includes Firmware update as well as Wi-Fi setup, and conditionally includes Weather.
