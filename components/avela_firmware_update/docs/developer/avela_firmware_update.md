# `avela_firmware_update`: network firmware updates

The firmware update component accepts an application image over the setup server, writes the inactive OTA slot, and
requests a restart. It also confirms a newly booted OTA image after the confirmation interval.

USB installation and settings preservation are described in [Build and flash](../../../../docs/developer/BUILD.md).
The [skin contract](SKIN.md) defines the screen's values and progress states.

## Opening and leaving the screen

A double click on Info opens Firmware update. Uploads are accepted only while that screen is open. Leaving it
disables uploads again. The screen refuses navigation while receiving an image or waiting to restart; after an
upload failure, it can be left normally.

Opening the update screen at the radio is the upload-acceptance gate. On the home network both routes also need the
[page password](../../../avela_setup/docs/developer/avela_setup.md#page-password), so an update needs someone at the
radio and the password.
Holding the board's BOOT button during reset starts the ESP32 flasher instead; that hardware behavior is described
in the [board reference](../../../avela_boards/docs/developer/avela_boards.md#supported-boards).

## Upload endpoint

`EspFirmwareUpdateService` registers two routes on `SetupServer`'s HTTP server:

| Route | Purpose |
|---|---|
| `GET /update` | Serve the upload page |
| `POST /update` | Receive the firmware image as the request body |

The page sends the raw file with fetch; `avela-tuner update FILE` uses the same upload path, with the page password
from `--password` or `$AVELA_PASSWORD`. Upload work runs on the
server's task, while the main loop continues copying progress into `FirmwareUpdateModel`.

## Validation and inactive-slot write

The receiver first collects enough bytes to read the image's app description. A short file is refused before an OTA
slot is touched. HTTP receives may return partial data, so the first read is accumulated until the header is
complete.

The app description must have the expected magic word and the running application's project name. These are image
compatibility checks. The service then writes to the inactive slot through esp_ota_* using a static 1024-byte chunk.

After all bytes arrive, `esp_ota_end` validates the image, including checksum and chip compatibility. A successful
image becomes the selected boot slot. Main-loop `update()` then schedules the restart.

## Failure handling and timing

| Condition | Result |
|---|---|
| File too short or wrong app description | Refuse before starting the slot write |
| Receive/write/final validation failure | Abandon the attempted image and publish a failure |
| Client stops sending | Give up after `setup_page::max_receive_timeouts` consecutive receive timeouts |
| Successful upload | Show restarting and restart after 1.5 seconds |

The shared [Setup](../../../avela_setup/docs/developer/avela_setup.md#limits-and-execution) defines the timeout
duration and consecutive-timeout limit. A stalled upload therefore cannot keep the single HTTP server task blocked
indefinitely. Its failed state also releases the screen's leave restriction.

## Restart and rollback confirmation

With `CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE` enabled, a new OTA image starts pending verification. The service
confirms it after 30 seconds of running with `esp_ota_mark_app_valid_cancel_rollback`. A crash or restart before
confirmation causes the bootloader to return to the previous image.

A USB-flashed image is not pending verification and needs no OTA confirmation. The
[build guide](../../../../docs/developer/BUILD.md) gives the settings-preserving flash procedure.

## Screen and tests

The screen shows the running version, upload addresses, and waiting/receiving/restarting/failed state. Exact setters
and ranges are in [SKIN.md](SKIN.md). Presentation, including official-skin progress-bar styling, belongs to the
skin.

`tests/avela_firmware_update_test/` covers the screen and model. It does not replace a board test of flash writes,
reset, or rollback.
