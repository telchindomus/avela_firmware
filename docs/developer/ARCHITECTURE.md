# Avela firmware: architecture

Avela runs the radio through a board, feature components, and a skin. The application in `main/` connects them: it
chooses the board and skin, owns the models and services, and runs the UI. Components expose their data and
interfaces without knowing which skin will draw them.

Read the [coding standard](CODING_STANDARD.md) for implementation rules, [Controls](FEATURES.md#controls) for
application behavior, and the [component index](../README.md#components) for individual references. Skin integration
and compatibility are defined in [Skin architecture](SKIN_ARCHITECTURE.md).

## Design rules

### `Application` lifetime and composition

`app_main` creates one function-local static `Application`. Its members include the board, band buffers, canvas,
fonts, models, services, setup frontends, and UI. These objects remain available throughout execution without
occupying the main task's stack. References are passed to their users; objects are not exposed as namespace globals.

The application wires its members by name. It initializes services, calls their updates, and constructs setup
frontends from the component sections. There is no self-registration or compile-time registry of components. The
service implementation is selected in `Application`, rather than by a runtime flag. Network-free simulated services
support UI development without a live stream.

`UiModels` passes const references to shared models into the application `Ui`. Its screen variant holds only the
active screen. A switch constructs each screen from its models and the chosen skin. Screen changes construct and
destroy the active alternative in place.

### Models, screens, and presentation

| Part | Responsibility |
|---|---|
| Data model | Shared component state for the duration of execution. Requested fields express what is wanted; reported fields describe what the service has observed. |
| Page model | State belonging to one open screen, such as its selected row. Its controller owns changes to that state. |
| Service | Takes requests and updates its data model on the main loop. Background work is handed back through the service. |
| Controller | Interprets commands for its screen and returns a request, a navigation response, or both. It may update its page model. |
| View | Reads models and passes concrete values to the skin during `draw()` and `refresh()`. |
| Skin | Draws those values and keeps the displayed values and animation state needed to decide when to redraw. |
| Screen | Combines a view, controller, and any page model. It is the unit constructed when navigating to a `ScreenId`. |

Controllers and views do not write shared data models. They keep no independent copy of requested or reported model
state. Page models and the skin's displayed-value cache have distinct purposes and lifetimes.

The component owns each screen's view, controller, and skin concept. Main chooses the page-skin type and supplies
the models. A screen is checked by `ScreenConcept`; the view template checks the page skin against its component
concept.

### Inputs, commands, and requests

An input event says what a physical input did and identifies its sender. The application converts that event into a
command. The active screen's controller gives the command its screen-specific meaning.

```text
board inputs -> input events -> Application command mapping -> active controller
                                                               |
                                    navigation response <------+-> service request
                                                                      |
                                                               service queue
                                                                      |
                                                              model update
                                                                      |
                                                              view -> skin
```

A `Response` can name a screen to open, close the current screen, and carry a service request. `Application` routes
requests to the appropriate service. Navigation and lifecycle hooks are described in the
[UI reference](../../components/avela_ui/docs/developer/avela_ui.md); the concrete mapping, screen tree, and leave
restrictions are in [Controls](FEATURES.md#controls).

### Persistent settings and service requests

Persistent configuration is stored through `Settings`. A service watches the revision of its record and follows a
newly saved value. This path handles configuration such as network credentials, station records, and weather
settings.

Transient actions go through the owning service's request queue. The knob, setup page, and serial console use the
same playback requests. A service takes them on the main loop and carries them out against its data model. Relative
actions, such as a volume increment or play/stop toggle, remain relative until processed, so queued actions
accumulate correctly.

Services publish status for clients on other tasks. Setup sections must use `Settings` or the service's
request/status interface rather than reaching into a mutable model. The
[messaging reference](../../components/avela_messaging/docs/developer/avela_messaging.md) defines queue ownership
and the [setup reference](../../components/avela_setup/docs/developer/avela_setup.md) defines section execution and
serialization.

### Main loop and background work

`Application` polls inputs and updates services on the main task. Views and skins draw on that same task. Exact
application scheduling is specified in [Controls](FEATURES.md#main-loop).

Blocking network and audio work runs outside the main loop. The radio uses receiver and player tasks with queues for
encoded audio and reports. Weather uses a worker task that publishes reports under a static mutex. Firmware uploads
run on the setup server's task and hand state back to the main loop. Atomics carry scalar state where appropriate.

Shared data models remain owned by the main loop. A callback or worker must use its service's synchronized handoff;
it must not write the model directly. Queues, mutexes, and task stacks use static storage in our code. See the
[coding standard](CODING_STANDARD.md#5-structure) for concurrency rules and each component reference for its actual
handoff.

### Hardware and memory boundaries

The HAL provides generic parts configured by descriptions. Boards own physical parts, wiring, and input identities.
Main knows the chosen board; feature components and skins do not include board-specific hardware. The
[board reference](../../components/avela_boards/docs/developer/avela_boards.md) distinguishes a module from an
assembled Avela board.

The firmware must fit the smallest supported board: avela piccolo, an ESP32-WROOM device with 4 MB flash and no
PSRAM. A larger board does not relax the firmware's memory constraints. The partition table is project
configuration; boards with different flash requirements need compatible defaults and partitions.

Our code uses fixed storage and compile-time sizes. Library-owned allocations, stack sizing, and measurement
requirements are covered by [Memory](CODING_STANDARD.md#3-memory-no-heap-in-our-code). Drawing uses the UI's
[band renderer](../../components/avela_ui/docs/developer/avela_ui.md#drawing); no framebuffer or sprite allocation
is used.

### Skins and optional components

A skin owns palette, fonts, assets, layout, wording, and motion. Behavior stays in controllers and services. The
header is outside the active screen and is shown by the application UI; it reads the clock, radio, and network. It
has no controller of its own.

Main links the chosen skin and checks its supported screen size. Component views check the required page skins at
compile time. The full build-selection, version, optional-component, and retirement contracts are in
[Skin architecture](SKIN_ARCHITECTURE.md); implementers follow [Writing a skin](SKINS.md).

An optional component is guarded explicitly in `Application`, `UiModels`, the screen variant/build switch, setup
sections, tile screen list, and skins. Its Kconfig switch controls sources and aliases. Its `REQUIRES` entries
remain unconditional. Retiring a component removes its code, integration, official-skin pages, and owned
documentation together.

### Clock

`SystemClock` is the shared entry point for local time and synchronization state. ESP-IDF's SNTP sets the system
clock; its callback only marks synchronization. Readers obtain time directly, rather than consuming published time
events.

The [clock reference](../../components/avela_clock/docs/developer/avela_clock.md) owns timezone behavior and the
callback contract. An additional time source requires an explicit setter that both sets the clock and marks it
synchronized; it must not duplicate SNTP's existing set operation.

### Dependency order

Dependencies point toward lower-level interfaces. The components with screens are `avela_radio`, `avela_network`,
`avela_system_info`, `avela_weather`, and `avela_firmware_update`. They do not depend on one another, except that
they may read the lower-level network model. Only main knows the complete screen set. No component depends on a
skin; skins depend on the concepts and value types they implement.

There is no general core component. A type shared by components lives in a component named for its concern. A helper
used by one component stays there until another component needs it. `avela_strings` supplies fixed-capacity text
below its users.

The table gives every component's direct local CMake requirements. Each dependency must remain below its user in the
dependency graph. Framework and external-library requirements are omitted. Public requirements propagate interfaces;
private requirements are implementation dependencies.

| Component | Local `REQUIRES` | Local `PRIV_REQUIRES` |
|---|---|---|
| `avela_boards` | `avela_hal`, `avela_settings`, `avela_setup` | — |
| `avela_clock` | `avela_strings`, `avela_settings`, `avela_setup` | — |
| `avela_firmware_update` | `avela_messaging`, `avela_strings`, `avela_ui`, `avela_network` | `avela_setup` |
| `avela_hal` | — | — |
| `avela_messaging` | — | — |
| `avela_network` | `avela_messaging`, `avela_strings`, `avela_ui`, `avela_setup` | `avela_clock`, `avela_settings` |
| `avela_radio` | `avela_strings`, `avela_messaging`, `avela_ui`, `avela_hal`, `avela_network`, `avela_setup`, `avela_settings` | — |
| `avela_settings` | `avela_strings` | — |
| `avela_setup` | `avela_strings` | `avela_settings` |
| `avela_skin_piccolo` | `avela_strings`, `avela_hal`, `avela_ui`, `avela_clock`, `avela_network`, `avela_radio`, `avela_system_info`, `avela_status_tray`, `avela_weather`, `avela_firmware_update` | — |
| `avela_skin_piccolo_silk_taupe` | `avela_strings`, `avela_hal`, `avela_ui`, `avela_clock`, `avela_network`, `avela_radio`, `avela_system_info`, `avela_status_tray`, `avela_weather`, `avela_firmware_update` | — |
| `avela_status_tray` | `avela_clock`, `avela_network`, `avela_radio` | — |
| `avela_strings` | — | — |
| `avela_system_info` | `avela_strings`, `avela_ui`, `avela_network` | — |
| `avela_ui` | `avela_messaging` | — |
| `avela_weather` | `avela_messaging`, `avela_strings`, `avela_ui`, `avela_setup`, `avela_network` | `avela_hal`, `avela_settings` |

Main composes the components through `main/CMakeLists.txt`. The top-level CMakeLists links the chosen skin into main
as described in [Skin architecture](SKIN_ARCHITECTURE.md#rules).

Changes to architecture rules or component dependencies require agreement before implementation, as specified in the
root instructions.

## Source layout

Public headers live in `components/<component>/include/<component>/`; implementation sources mirror their topics
under `src/`. Includes use `"<component>/<file>.hpp"`. A type has its own file, named after the type in snake_case.

| Location | Contents |
|---|---|
| `main/` | `Application` lifetime, board/skin choice, UI composition, routing, and loop |
| `components/` | Component code and its README, developer references, and skin contracts |
| `tests/` | Separate ESP-IDF Linux test projects for platform-free code |
| `tools/` | Code, documentation, and skin checks; host tests; committed-asset generation |
| `cmake/strict_warnings.cmake` | Shared strict-warning setup for firmware and test projects |
| `partitions.csv` | Project flash layout |

Use the [component index](../README.md#components) to find the owner of an implementation detail. Component entry
points link to the code topics that need explanation; this source map does not repeat a per-file inventory.
