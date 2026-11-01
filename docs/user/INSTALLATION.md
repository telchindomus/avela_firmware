# Install or recover the firmware

Use this page for a supported CYD-based Avela Piccolo that needs its first firmware installation or a USB recovery.
For a radio that already runs Avela, use
[Firmware updates](../../components/avela_firmware_update/docs/user/UPDATE.md) first.

These instructions use prebuilt firmware files. Choose the package for your hardware and desired skin; a skin is the
screen's appearance. Building from source is covered separately in the developer documentation.

## Choose the right file

| File | Purpose |
|---|---|
| `avela_firmware.bin` | Application image for a network update, or a USB update together with OTA metadata |
| `ota_data_initial.bin` | Selects the application written by the settings-preserving USB procedure |
| `avela_firmware_full.bin` | Complete first-install image; writing it erases saved settings |

Use files from the same firmware package and skin. Release builds provide `checksums.sha256` beside the skin
folders; compare your files with that manifest before flashing.

A complete first-install image writes over the settings area. If the radio still runs and you need its settings,
make a [backup with secrets](../../components/avela_setup/docs/user/USB.md#back-up-settings) before using that
image.

## Prepare the USB connection

Use the display board's USB data connection and a data-capable cable. A power-only socket or cable cannot flash the
board. Close avela-tuner, serial terminals and any other program using the port.

Install Espressif's flashing tool using its
[installation instructions](https://docs.espressif.com/projects/esptool/en/latest/esp32/installation.html). The
commands below use esptool 5's `esptool` and `write-flash` spellings. Espressif's
[migration guide](https://docs.espressif.com/projects/esptool/en/latest/esp32/migration-guide.html) explains the
change from esptool 4's `esptool.py write_flash`.

Replace `<PORT>` with your board's serial port, such as `COM4` on Windows, `/dev/ttyUSB0` on Linux or the
appropriate `/dev/cu.*` device on macOS. Run the command from the folder containing the firmware files.

## First installation or full recovery

This command writes the full image at `0x0` and erases saved settings:

```sh
esptool --chip esp32 --port <PORT> write-flash 0x0 avela_firmware_full.bin
```

Keep the USB connection in place until the tool finishes. After flashing, let the board restart. If it remains in
flashing mode, release BOOT and press reset. Continue with [First start](FIRST_START.md), or restore your saved
settings through USB.

The full image also erases the page password, which a settings backup cannot restore.
[Set a new one over USB](../../components/avela_setup/docs/user/USB.md#set-the-page-password-over-usb), or set it
on Access during first-start Wi-Fi setup.

## USB update that keeps settings

For an already installed radio with the supported partition layout, write the application and its initial OTA
metadata together:

```sh
esptool --chip esp32 --port <PORT> write-flash 0xd000 ota_data_initial.bin 0x10000 avela_firmware.bin
```

This procedure preserves saved settings. Writing the application at `0x10000` alone is insufficient: the radio may
still be set to boot its other application slot. Do not replace the two-file command with the full-image command
when you want to keep settings.

## If the radio does not start

If esptool cannot connect, check the data cable, selected port and whether another program has the port open. If
necessary, enter flashing mode using
[BOOT and reset](../../components/avela_boards/docs/user/DEVICE.md#boot-and-reset), then retry.

If flashing finished but Avela does not start, check that BOOT is released. Confirm you used the correct hardware
package and one complete procedure above. An application-only write may leave the old boot selection in place; use
the application-plus-metadata procedure to correct it. Use the full image only when a full recovery is needed and
settings loss is acceptable.

After the radio starts, Info shows the running version. A version ending in `-dev` is a development build.
