# `avela_network` for skin authors

The Wi-Fi setup page displays the saved home network and the device's setup-network connection details. The network
service opens that setup AP only when requested; see [Network behavior](avela_network.md).

## `Skin::WifiSetupSkin` (fulfils `WifiSetupSkinConcept`)

Construct as `Skin::WifiSetupSkin{surface}`.

| Setter | Values | Meaning |
|---|---|---|
| `set_saved_network(status, ssid)` | `WifiStatus`, text | Saved home-network state; may be unconfigured |
| `set_setup_network(name, password, join_code)` | Three texts | Setup SSID, password, and QR payload for joining |
| `set_setup_page(active, address)` | bool, text | Whether setup is ready and its page address |
| `draw()` | — | Entire content area |

Until active is true, show that setup is starting. Do not offer a QR code or connection details that are not ready
to use. Render join_code with `Painter::qr_code(text, area, first_version, dark, light, corner_radius)` if using the
supplied QR helper. Dark modules must remain clearly darker than light modules so phones can scan it.

The [control table](../../../../docs/developer/FEATURES.md#commands-on-each-screen) defines Back and playback
behavior. Follow the
[shared drawing lifecycle](../../../avela_ui/docs/developer/SKIN.md#drawing-rules-for-every-skin).

The concept is `include/avela_network/ui/wifi_setup_skin_concept.hpp`; official wifi_setup_skin implementations show
a reference appearance.
