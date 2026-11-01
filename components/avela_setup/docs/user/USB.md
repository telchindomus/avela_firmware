# USB configuration and backups

The avela-tuner application configures the radio from a computer over a USB data connection. It offers a terminal
interface and commands for changing settings, controlling playback, and backing up or restoring settings. The radio
can be configured this way before it has Wi-Fi.

The instructions below assume avela-tuner is installed. It is a separate application from the firmware. To install a
supplied wheel with pipx, use `pipx install <path-to-avela-tuner-wheel.whl>`. It requires Python 3.11–3.14. The
package's own instructions cover installation from its source checkout.

## Connect the radio

Use the display board's USB data port and a data-capable cable. A power-only connection cannot carry settings.
Follow your device's hardware instructions to access its data connector.

Close any serial monitor or other program using the same port. Replace `<PORT>` in the examples with your device's
port: for example `COM4`, `/dev/ttyUSB0` or the appropriate `/dev/cu.*` device. To check the connection, run:

```sh
avela-tuner --port <PORT> sections
```

The result lists the sections and fields supported by the connected radio. With no port option, the tool uses
`AVELA_PORT` if set, then `/dev/ttyUSB0`. The default serial speed is 115200 baud. Use a tuner that supports the
installed firmware's console protocol; the firmware described here uses protocol 2.

## Use the terminal interface

Run the tool without a command:

```sh
avela-tuner --port <PORT>
```

Choose a section from the list. Tab moves between the section list and its fields; the arrow keys move within them.
Edit a value and press F2 to save. Only changed fields are sent. Leaving the password field unchanged keeps the
saved password; this differs from saving the
[browser's Wi-Fi form](../../../avela_network/docs/user/WIFI.md#change-the-home-network).

| Key | Action |
|---|---|
| F2 | Save changes |
| F5 | Reload values |
| F6 | Dump settings to a file; choose whether to include secrets |
| F7 | Restore settings from a file |
| F8 | Restart the radio |
| F10 | Quit |
| Enter in a dialog | Confirm |
| Escape in a dialog | Cancel |

A refused value has an error beneath its field. Correct it and save again. Reload to see values changed through
another frontend.

## Change a setting from the command line

Read a section, then set the fields you want to change. Values omitted from a command keep their saved values:

```sh
avela-tuner --port <PORT> get weather
avela-tuner --port <PORT> set weather place="New York" units=imperial
```

Quote a value containing spaces according to your computer's shell. These commands choose volume and playback
without editing settings forms:

```sh
avela-tuner --port <PORT> volume 30
avela-tuner --port <PORT> play
avela-tuner --port <PORT> stop
```

`play <slot>` selects a station slot and starts it. Use the field names reported by `sections` for other changes.
Feature guides describe the values and limits; unknown sections or fields are refused.

## Set the page password over USB

USB lets you set or replace the page password without knowing the old one. Replace `<NEW_PASSWORD>` with a password
that follows the [page-password rules](SETUP.md#set-or-reset-the-page-password):

```sh
avela-tuner --port <PORT> set access password="<NEW_PASSWORD>"
avela-tuner --port <PORT> get access
```

`get access` reports only whether a page password is set. It returns neither the password nor its hash, even when
you ask for secrets. The terminal interface's Access section also lets you enter a new password; an unchanged,
empty password field keeps the existing one.

## Back up settings

Dump settings without secret fields:

```sh
avela-tuner --port <PORT> dump -o avela-settings.json
```

For a backup that also preserves the Wi-Fi password, include secrets:

```sh
avela-tuner --port <PORT> dump -o avela-settings-private.json --secrets
```

Keep the second file private: it contains the Wi-Fi password as readable text. The tool restricts the file to its
owner where file permissions support that. A backup without secrets cannot supply the missing password after
settings have been erased.

The page password is never included, even with `--secrets`. Set or replace it separately using the
[USB procedure above](#set-the-page-password-over-usb).

The file contains the radio's reported settings and playback choices. It is not a copy of the firmware, the skin, or
transient display data such as song titles and forecasts. Before a full USB reinstallation, keep a backup with
secrets if you want to restore Wi-Fi too.

## Restore settings

With the radio running and connected over USB:

```sh
avela-tuner --port <PORT> restore avela-settings-private.json
```

Read the result for every section. A restore continues after a section fails; it is not one all-or-nothing
transaction. Correct reported errors and check the resulting settings. Fields absent from the file keep their
current values. A file without the Wi-Fi password therefore preserves a password that still exists, but cannot
recover one erased by reinstallation.

Restoring a tuner backup leaves the radio's page password unchanged. After a full reinstallation that erased it,
set a new one over USB; the backup cannot recover the old page password.

If the restore changes the radio name, [restart](../../../avela_boards/docs/user/DEVICE.md#restart-the-radio) to
apply that name. A saved home network closes the temporary setup network just as it does from the browser.

## Connection problems

If no radio answers, check the selected port, data cable and data connector, and close other programs using that
port. Confirm Avela is running rather than waiting in the ESP32 flasher. Release BOOT and reset if needed. If the
port cannot be opened, check your operating system's serial-device permissions and USB driver.

A protocol mismatch needs a compatible tuner and firmware pair. Messages about an unknown field or invalid value are
different: the connection worked, and the radio refused that setting. Read the reported field and reason.

The tuner's `update` command sends firmware over the network, even when its other commands use USB. Follow
[Firmware updates](../../../avela_firmware_update/docs/user/UPDATE.md#update-with-avela-tuner) for that procedure.
