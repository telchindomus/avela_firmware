# Skin changes

This reference lists required skin changes for firmware versions that advance `supported_skin_api_versions`, newest
first. Removals are also listed as code an author may delete. See
[Versions and skin compatibility](SKIN_ARCHITECTURE.md#versions-and-skin-compatibility) for the version policy.

## 0.1.0

The initial skin API requires the following interfaces:

| Area | Implement | Contract |
|---|---|---|
| Shared UI | Skin, Fonts, Surface, screen, `TilesSkin` | [UI](../../components/avela_ui/docs/developer/SKIN.md) |
| Header | `HeaderSkin` | [Status tray](../../components/avela_status_tray/docs/developer/SKIN.md) |
| Radio | `PlayerSkin`, `StationsSkin` | [Radio](../../components/avela_radio/docs/developer/SKIN.md) |
| Device information | `InfoSkin` | [System info](../../components/avela_system_info/docs/developer/SKIN.md) |
| Network setup | `WifiSetupSkin` | [Network](../../components/avela_network/docs/developer/SKIN.md) |
| Firmware update | `FirmwareUpdateSkin` | [Firmware update](../../components/avela_firmware_update/docs/developer/SKIN.md) |
| Optional Weather | `WeatherSkin` and its tile | [Weather](../../components/avela_weather/docs/developer/SKIN.md) |
