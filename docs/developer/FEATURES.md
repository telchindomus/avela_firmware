# Avela firmware: application controls

The application maps physical inputs to commands and decides which screen is active. Each screen's controller
handles those commands according to its purpose. Component behavior is documented in the
[component references](../README.md#components).

## Controls

### Input mapping

The application converts encoder and Back button events into the same command types on every screen. Turning the
encoder changes volume on the player screen and selection on the Stations screen because their controllers handle
that command differently.

| Physical input | Command |
|---|---|
| Encoder rotation | `IncrementCommand` or `DecrementCommand`; `m_steps` is the number of detents |
| Encoder click | `OkCommand` |
| Encoder double click | `MenuCommand` |
| Encoder long press | `OptionsCommand` |
| Back button click | `BackCommand` |

The HAL detects physical events and their senders; it assigns no application meaning. Its
[input reference](../../components/avela_hal/docs/developer/avela_hal.md#inputs) defines debounce and button timing.
`Application` compares senders with the chosen board's inputs before making commands.

### Screen navigation

The tiles are the root of the screen tree. Startup opens the player, with the path `tiles -> player`. Main supplies
the tile list: Radio, optional Weather, and Info.

```text
Tiles
├── Radio / player ── Stations
├── Weather (when enabled)
└── Info ── Wi-Fi setup
         └─ Firmware update
```

The UI keeps the path of open `ScreenId`s, with capacity for four levels. A controller opens a child through
`Response::m_show`. Back closes the active screen and returns to its parent; at the root it does nothing. A
controller can also close its screen through `Response::m_close`.

Before any screen change, the controller's leave permission and lifecycle hook are checked. Firmware update prevents
leaving while receiving or restarting. When returning to the tiles, the tile for the screen just closed is selected.
On first opening, the selection is Radio.

Adding a `ScreenId` requires updating the application's screen-building switch and official skins' tile handling.
Their exhaustive handling is checked by the build.

### Commands on each screen

`Menu` means encoder double click; `Options` means encoder long press. Unlisted commands have no effect.

| Screen | `Turn` | `Click` | Menu | Options | Back |
|---|---|---|---|---|---|
| Tiles | Choose a tile, without wrapping | Open selected screen | — | — | Stay on tiles |
| Player | Volume | Play/stop | Open Stations | — | Tiles |
| Stations | Move selection | Play selection and close | — | — | Player |
| Weather | Volume | Play/stop | — | Refresh weather | Tiles |
| Info | Volume | Play/stop | Open Firmware update | Open Wi-Fi setup | Tiles |
| Wi-Fi setup | Volume | Play/stop | — | — | Close setup network; return to Info |
| Firmware update | Volume | Play/stop | — | — | Return to Info, except while receiving/restarting |

Playback controls are available wherever the encoder has no other screen-specific job. Volume changes by two units
per detent; click toggles requested playback. `PlaybackControls` implements these shared controller handlers, and
the header shows their effect.

Opening Wi-Fi setup requests the setup network; leaving requests its closure. Saving home-network credentials closes
it too. If that closure occurs while Wi-Fi setup is shown, the application returns to Info. Opening Firmware update
enables uploads; leaving disables them.

### Main loop

Inputs are polled every **5 ms**. Controller requests are queued as they are handled. Every **33 ms**, services take
their requests and update their models; the radio publishes its status, and the active view and shared header
refresh.

The setup page and console execute on their own tasks. Their playback sections use the same radio request/status
interface as the encoder path. Configuration sections save `Settings` records whose revisions are followed by their
services. These paths are defined in the
[setup reference](../../components/avela_setup/docs/developer/avela_setup.md).

### Routing and lifecycle

`Application` converts input events in `command_for()` and passes commands to `Ui::handle(command, route)`. Back is
handled by `Ui`. Other commands are dispatched only to the active controller; a missing overload means no action,
without runtime subscription lists.

A controller response is routed to the service before navigation is performed. Requests from `shown()` and `left()`
follow the same route. The [UI reference](../../components/avela_ui/docs/developer/avela_ui.md) explains the
reusable hooks; main owns the concrete path and screen construction.
