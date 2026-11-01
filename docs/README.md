# Avela firmware documentation

Start with the guide for what you want to do. User documentation explains how to operate an installed radio.
Developer documentation explains how to change or build the firmware. Feature instructions live with their owning
components.

## User documentation

[The user guide](user/README.md) is the entry point for first connection, controls, stations, settings, backups,
updates and troubleshooting. [First start](user/FIRST_START.md) covers an assembled radio;
[Installation](user/INSTALLATION.md) covers prebuilt firmware installation and USB recovery.

## Developer documentation

| Task | Start here |
|---|---|
| Understand or change the firmware | [Architecture](developer/ARCHITECTURE.md), then [Coding standard](developer/CODING_STANDARD.md) |
| Build, flash, or run checks | [Build and flash](developer/BUILD.md) |
| Change application controls or navigation | [Controls](developer/FEATURES.md#controls) |
| Release the firmware or build release images | [Releases and versions](developer/RELEASE.md) |
| Write a skin | [Writing a skin](developer/SKINS.md) |
| Change the skin integration or compatibility rules | [Skin architecture](developer/SKIN_ARCHITECTURE.md) and [Skin changes](developer/SKIN_CHANGES.md) |
| Refresh user-guide illustrations | [User-documentation illustrations](developer/USER_DOC_IMAGES.md) |
| Write or edit documentation | [Writing standard](WRITING_STANDARD.md), then [Editing workflow](AGENTS.md) |

## Components

| Component | Responsibility |
|---|---|
| [Boards](../components/avela_boards/README.md) | Board descriptions, hardware ownership, wiring, and board settings |
| [HAL](../components/avela_hal/README.md) | Generic hardware parts and input events |
| [UI](../components/avela_ui/README.md) | Commands, controllers, drawing foundation, and tiles |
| [Messaging](../components/avela_messaging/README.md) | Service requests and published status |
| [Settings](../components/avela_settings/README.md) | Persistent records and atomic updates |
| [Setup](../components/avela_setup/README.md) | Web and serial frontends for component settings |
| [Network](../components/avela_network/README.md) | Wi-Fi, setup network, and network discovery |
| [Clock](../components/avela_clock/README.md) | Local time, synchronization state, and timezone settings |
| [Radio](../components/avela_radio/README.md) | Stations, streaming, decoding, and playback |
| [Weather](../components/avela_weather/README.md) | Place lookup and weather reports |
| [Firmware update](../components/avela_firmware_update/README.md) | Network upload, OTA installation, and rollback confirmation |
| [System info](../components/avela_system_info/README.md) | Memory, task stacks, firmware version, and uptime |
| [Status tray](../components/avela_status_tray/README.md) | Shared header data |
| [Strings](../components/avela_strings/README.md) | Fixed-capacity text without heap allocation |
| [Piccolo skin](../components/avela_skin_piccolo/README.md) | Default deep teal skin |
| [Silk Taupe skin](../components/avela_skin_piccolo_silk_taupe/README.md) | Taupe, espresso, and champagne gold skin |
