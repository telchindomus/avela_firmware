# Troubleshooting

Start with the message on the screen. If you can open Info, check its network and address rows before changing
station or weather settings. A radio that has not joined Wi-Fi cannot receive an internet stream or a forecast.

| Symptom | What to check |
|---|---|
| Tapping the screen does nothing | Use the knob and Back button; see [Controls](CONTROLS.md). |
| Waiting for Wi-Fi, or no home-network address | Follow [If connection fails](../../components/avela_network/docs/user/WIFI.md#if-connection-fails). |
| The setup network is absent | Open Wi-Fi setup explicitly; see [Connect for the first time](../../components/avela_network/docs/user/WIFI.md#connect-for-the-first-time). |
| The phone disconnects after saving Wi-Fi | This is expected when the radio closes setup; see [After saving](../../components/avela_network/docs/user/WIFI.md#after-saving). |
| The setup page will not open | Use the address and checks in [If the page does not open](../../components/avela_setup/docs/user/SETUP.md#if-the-page-does-not-open). |
| The page asks for a password I do not know | [Set a new page password](../../components/avela_setup/docs/user/SETUP.md#set-or-reset-the-page-password) from the radio's setup network or over USB. |
| The setup network also asks for the page login | The home and setup address ranges may overlap. See [Page access and USB recovery](../../components/avela_setup/docs/user/SETUP.md#set-or-reset-the-page-password). |
| Stopped, silent playback, or a station error | Follow [If a station does not play](../../components/avela_radio/docs/user/RADIO.md#if-a-station-does-not-play). |
| Station edits do not appear on the device | See [After editing](../../components/avela_radio/docs/user/RADIO.md#after-editing). |
| Weather is absent, stale or for the wrong place | See [Weather messages and recovery](../../components/avela_weather/docs/user/WEATHER.md#messages-and-recovery). |
| Time is blank or the hour is wrong | See [Clock problems](../../components/avela_clock/docs/user/CLOCK.md#clock-problems). |
| Brightness or restart needs changing | Use [Device settings](../../components/avela_boards/docs/user/DEVICE.md). |
| USB configuration cannot find the radio | Follow [USB connection problems](../../components/avela_setup/docs/user/USB.md#connection-problems). |
| A firmware upload is refused or the old version returns | See [Update problems](../../components/avela_firmware_update/docs/user/UPDATE.md#update-problems). |
| The radio will not start after USB flashing | Follow [If the radio does not start](INSTALLATION.md#if-the-radio-does-not-start). |

## Before restarting

If the radio still responds, note the selected station, the exact message, and the network and firmware details on
Info. Those details make a repeatable problem easier to describe. Use the
[restart action](../../components/avela_boards/docs/user/DEVICE.md#restart-the-radio) when you need to restart
without unplugging power.

For a recurring problem, record what you did immediately before it, whether it affects one station or all stations,
and whether the setup page still responds. Do not include Wi-Fi passwords or a backup containing secrets in a public
report. Keep the page password private too.
