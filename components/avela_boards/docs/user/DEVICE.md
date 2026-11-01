# Device settings

The Board tab on the setup page controls screen brightness and provides a restart action. These settings apply to
the assembled board, independently of the selected station.

## Change brightness

1. Open the [setup page](../../../avela_setup/docs/user/SETUP.md) and choose Board.
2. Enter Screen brightness (%) as a whole number from 10 to 100.
3. Press Save for that form.

The brightness changes without restarting and is saved for later starts. The default is 80 percent. The minimum
keeps the backlight on; 0 is not an accepted brightness setting.

## Restart the radio

Open Board and press Restart. The radio schedules a restart after answering the browser. Allow about 10 seconds
before reloading the page, as its message advises. The connection and address may take longer to return if Wi-Fi is
unavailable.

A restart keeps saved settings; it does not reset the radio to its defaults. The
[USB tool](../../../avela_setup/docs/user/USB.md) also provides a restart action.

## BOOT and reset

On the supported CYD-based Piccolo, BOOT acts as Back while Avela is running. Reset restarts the device. The names
describe different actions; use Back for ordinary navigation.

Holding BOOT while resetting places the ESP32 in its USB flasher instead of starting Avela. For a manual flash
connection, hold BOOT, press and release reset, then release BOOT once the flashing tool connects. After flashing,
release BOOT and reset again if the application has not started.

If the device unexpectedly waits in flashing mode, check that BOOT is not held down when resetting or reconnecting
power. Physical button and socket positions depend on the enclosure; use the labels on your device.

[Install or recover the firmware](../../../../docs/user/INSTALLATION.md) gives the file choices and USB procedures.
Hardware assembly and wiring belong to the board's developer reference rather than this operating guide.
