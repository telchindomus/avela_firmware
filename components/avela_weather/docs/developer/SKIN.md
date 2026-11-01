# `avela_weather` for skin authors

Weather displays current conditions, today's high/low, and four following days for the configured place. The
[component reference](avela_weather.md) owns refresh, settings, publication, and optional-build behavior.

Guard Weather page sources with `if(CONFIG_AVELA_WEATHER)` and its alias with the corresponding C++ guard. Keep the
component `REQUIRES` unconditional, following
[Skin architecture](../../../../docs/developer/SKIN_ARCHITECTURE.md#rules).

## `Skin::WeatherSkin` (fulfils `WeatherSkinConcept`)

Construct as `Skin::WeatherSkin{surface}`.

| Setter | Values | Meaning |
|---|---|---|
| `set_weather(const WeatherReport&)` | Complete report | Called each frame; redraw when the report revision differs |
| `set_host_name(host_name)` | Text | Effective radio name used in the configuration hint |
| `draw()` | — | Entire content area |

## `WeatherReport`

| Field | Value |
|---|---|
| `m_status` | not_configured, waiting_for_network, loading, ready, place_not_found, failed |
| `m_place` | User-entered place, up to 40 bytes of text |
| `m_units` | metric or imperial; supplied measurements already use the selected units |
| `m_has_values` | Whether the measurements come from a successful update; values may be older than the current status |
| `m_temperature`, `m_feels_like` | Whole °C or °F |
| `m_condition` | clear, partly_cloudy, cloudy, fog, drizzle, rain, snow, thunderstorm |
| `m_daytime` | Bool; a moon may replace the sun at night |
| `m_humidity_percent` | 0–100 |
| `m_wind_speed` | Whole km/h or mph |
| `m_high`, `m_low` | Today's whole-degree limits |
| `m_forecast` | Next four days: weekday 0=Sunday, condition, high, and low |
| `m_revision` | Changes when a report is published, including status transitions |

Without values, explain the status: no place configured, waiting for Wi-Fi, loading, place not found, or request
failed. For no place, point to `http://<host_name>.local`. With older values, keep displaying them and indicate the
current status. Do not assume revisions are separated by the forecast interval.

The application reaches Weather from its tile and supports a manual refresh; see
[Controls](../../../../docs/developer/FEATURES.md#commands-on-each-screen). `TilesSkin` must draw an icon for
`ScreenId::weather` when enabled.

The concept and values are in `include/avela_weather/ui/weather_skin_concept.hpp` and
`include/avela_weather/model/weather_report.hpp`. The
[shared drawing lifecycle](../../../avela_ui/docs/developer/SKIN.md#drawing-rules-for-every-skin) applies.

## Reference look

Both official skins draw weather icons from shapes through weather_icons and use a 40 px display font role for
digits, minus, and degree. Their weather_skin implementations provide layouts; those visual choices are not
additional service behavior.
