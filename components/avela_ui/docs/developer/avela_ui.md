# `avela_ui`: UI interfaces and drawing

The UI component provides commands, controller hooks, screen/view concepts, tiles, fonts, and the canvas. The
application `Ui` class lives in `main/ui.hpp`; it chooses the concrete screens and navigation path. The
[application reference](../../../../docs/developer/FEATURES.md#controls) owns that composition.

The [shared skin contract](SKIN.md) describes the surface and page-skin interfaces used by skin authors.

## Commands and controller responses

Command is a variant of `IncrementCommand`, `DecrementCommand`, `OkCommand`, `BackCommand`, `MenuCommand`, and
`OptionsCommand`. `Application` makes those commands from physical events; they do not carry a board dependency.

A controller supplies `handle(command-type)` overloads for commands it understands. `handle_command()` selects the
overload at compile time for each variant alternative; a missing overload returns an empty `Response`. `Response`
can contain `m_show`, `m_close`, and `m_request`. A controller can also change its page model without navigating or
requesting work.

`PlaybackControls` provides common volume and play/stop handlers to controllers whose encoder has no other job.
Derived controllers expose those overloads with `using PlaybackControls::handle;`.

## Lifecycle hooks

| Optional controller hook | Meaning |
|---|---|
| `shown()` | Return a Request when the screen opens |
| `left()` | Return a Request when the screen closes |
| `can_leave()` const | Permit or refuse leaving |

`controller_calls.hpp` checks these optional interfaces. `shown_request()` and `left_request()` return no request
when the hook is absent; `may_leave()` defaults to true. The application routes any returned request through the
owning service interface.

`ScreenConcept` checks a screen's parts. A page model holds only screen-local state, such as selection; the view
reads it const and the controller changes it. The active screen's lifetime ends when it is replaced.

## Drawing

`Canvas` draws an area through `Painter` using screen coordinates. `Painter` translates those coordinates to the
current band. `Canvas` renders into two buffers and sends completed bands with DMA while drawing the next band.
There is no framebuffer and no heap allocation for drawing.

`band_lines` in `include/avela_ui/band_buffers.hpp` is **10**. At 320 pixels wide and RGB565, each buffer is 6400
bytes, or 6.25 KiB; the pair is 12,800 bytes, or 12.5 KiB. A 170-line full-screen render takes 17 bands. Partial
updates render only the affected area.

The callback passed to `Canvas::render()` runs once for each band. It must describe the same picture throughout that
render: compute animation state outside the callback, rather than advancing it per band. The render area must fit
the buffer width.

Views call the skin's setters to provide values, then `draw()` for the initial image; subsequent refreshes provide
current values. Skins cache displayed values and redraw changed visible output. Animation steps can change visible
output even when the supplied data is equal. All drawing runs on the main task: the canvas and LovyanGFX are not
thread-safe.

## Tiles and screen names

The component owns `TilesModel`, `TilesController`, `TilesView`, and `TilesScreen`. Main supplies the enabled tile
screen list in order. A skin draws the selected tile; it does not decide the list or navigation behavior.

`ScreenId` names application screens because a controller can open another screen. Optional entries are guarded by
their feature switch. `include/avela_ui/screen_id.hpp` is the authoritative enum definition.

The [application reference](../../../../docs/developer/FEATURES.md#screen-navigation) describes startup, Back, path
capacity, and selection on return to the tiles. `tests/avela_ui_test/` covers platform-free UI logic.
