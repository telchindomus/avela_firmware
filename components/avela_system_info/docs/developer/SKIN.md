# `avela_system_info` for skin authors

The Info page displays network and device facts. [System information](avela_system_info.md) explains measurement
semantics; the skin formats them for the screen.

## `Skin::InfoSkin` (fulfils `InfoSkinConcept`)

Construct as `Skin::InfoSkin{surface}`.

| Setter | Values | Meaning |
|---|---|---|
| `set_network(status, ssid, address, host_name, signal_dbm)` | `WifiStatus`, three texts, dBm | Address/signal are meaningful only while connected; page host is `http://<host_name>.local` |
| `set_memory(free, lowest_free, total)` | Internal RAM in units of 1024 bytes | Current free, lowest free since startup, and total |
| `set_firmware(version, uptime)` | Text, `std::chrono::seconds` | Running version and time since startup |
| `draw()` | — | Entire content area |

Always show Network, Address, Signal, Memory, and Firmware rows, with a placeholder where a value does not apply.
Show hints that holding the encoder opens Wi-Fi setup and double click opens Firmware update. Exact commands and
Back behavior are in the
[application control table](../../../../docs/developer/FEATURES.md#commands-on-each-screen).

Official skins show `Not configured` for an unset network, an en dash for unavailable values, free memory as a
percentage, and `Hold: Wi-Fi setup · Double-click: update` as the hint. They are examples of formatting these
values.

The concept is `include/avela_system_info/ui/info_skin_concept.hpp`; official info_skin implementations provide
reference layouts. Follow the
[shared drawing lifecycle](../../../avela_ui/docs/developer/SKIN.md#drawing-rules-for-every-skin).
