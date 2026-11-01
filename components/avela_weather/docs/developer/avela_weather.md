# `avela_weather`: weather reports

Weather retrieves current conditions, today's high and low, and the next four days from Open-Meteo over HTTPS. No
API key is required. ESP-IDF's certificate bundle checks the connection.

The [skin contract](SKIN.md) defines report fields and rendering states. `Application` commands are listed in
[Controls](../../../../docs/developer/FEATURES.md#controls).

## Configuration and refresh

`WeatherSection` accepts a place and metric or imperial units through the setup page or console. It also shows the
location returned by the lookup. The service looks up coordinates for a place and stores them in `WeatherSettings`,
then requests forecasts using those coordinates.

| Trigger | Action |
|---|---|
| Regular interval | Refresh every 15 minutes by default; `AVELA_WEATHER_REFRESH_MINUTES` sets the interval |
| `Settings` change | Make a refresh due immediately |
| Long press on Weather | Request a refresh |
| Joining the network | Make a refresh due immediately |
| Request failure | Retry after one minute |

Requests run on the worker, so “immediately” means without waiting for the regular interval; it does not interrupt a
request already running. Successful coordinates avoid repeated geocoding for the unchanged place. A failed lookup
can be retried, and coordinates that cannot be saved are retained for the current run.

## `Settings` changed during a request

The service stores coordinates only if the saved place still matches the place it looked up and is not already
located. `Settings::update()` and `take_location()` merge that result with the current record, preserving other
changes such as units. If another place was saved, the service follows the new record on its next update.

A report belongs to one place and unit system. `match_report()` clears values belonging to different settings before
the new report is published. Clearing a place also clears its forecast. The cleared report must be published even
when its status equals the next waiting status, so the display cannot retain old values.

`WeatherSection` also uses `Settings::update()`. Omitted fields keep their current stored values, and an unchanged
place keeps its coordinates. A frontend save therefore does not undo a coordinate save or another field change that
completed while the request was being prepared.

## Worker and report publication

`OpenMeteoWeatherService` performs HTTPS on its own task with an 8192-byte static stack; TLS uses that stack too.
Atomics pass network availability and refresh requests to it. The worker publishes `WeatherReport` under a static
mutex, and `update()` copies it into `WeatherModel` on the main loop.

A report revision can change on status transitions as well as forecast updates. A loading report and its result can
follow quickly; the forecast interval is not a minimum time between revisions. Skins check the revision whenever
`set_weather()` is called.

`open_meteo.*` parses JSON in place without heap allocation or platform headers. ESP-IDF's HTTP client and TLS
allocate request resources and release them afterward. The parser is exercised by `tests/avela_weather_test/`.

## Optional build integration

`CONFIG_AVELA_WEATHER` enables the component and defaults to on. Disabling it excludes Weather's sources and
integration. The measured default configuration's image is about 41 KB smaller; that size depends on configuration.
HTTP and TLS remain available for radio streams.

The switch guards `ScreenId::weather`, `UiModels::m_weather`, the `Ui` variant/build switch, the tile list,
`Application`'s model/service/setup section, and each skin's Weather sources and alias. `REQUIRES` stays
unconditional, following [Skin architecture](../../../../docs/developer/SKIN_ARCHITECTURE.md#rules), including rule
6 for the tile screen list.

`WeatherSkinConcept` takes a whole report through `set_weather()` and the radio's host name through
`set_host_name()`. The component's [skin reference](SKIN.md) owns the complete drawing contract and official-skin
rendering example.
