# `avela_firmware_update` for skin authors

The Firmware update screen shows upload addresses and installation progress. The
[component reference](avela_firmware_update.md) defines acceptance, errors, restart, and rollback behavior.

## `Skin::FirmwareUpdateSkin` (fulfils `FirmwareUpdateSkinConcept`)

Construct as `Skin::FirmwareUpdateSkin{surface}` and format the supplied values.

| Setter | Values | Meaning |
|---|---|---|
| `set_running_version(version)` | Text | Currently running firmware |
| `set_update_page(by_host_name, by_address)` | Two texts | Full page URL by host name and connected IP; IP URL is empty when unavailable |
| `set_progress(state, percent, failure)` | `FirmwareUpdateState`, 0–100, text | waiting, receiving, restarting, failed; failure is a short explanation |
| `draw()` | — | Entire content area |

Show where to upload while waiting, progress while receiving, and the state in words. Only show the Back hint while
waiting or failed. Back does nothing while receiving or restarting. Other application controls are in the
[control table](../../../../docs/developer/FEATURES.md#commands-on-each-screen).

`InfoSkin` must hint that a double click opens Firmware update, as defined in its
[contract](../../../avela_system_info/docs/developer/SKIN.md). The update address is
`http://<host_name>.local/update`; avela-tuner can upload through the same service.

The concept is `include/avela_firmware_update/ui/firmware_update_skin_concept.hpp`. Official firmware_update_skin
implementations provide reference appearance, including Silk Taupe's gold-highlighted progress bar. Follow the
[shared drawing lifecycle](../../../avela_ui/docs/developer/SKIN.md#drawing-rules-for-every-skin).
