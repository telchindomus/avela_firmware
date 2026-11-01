# Setup page

The setup page lets you control playback and change settings from a browser on your phone or computer. Your radio
and browser device must share a reachable network, either your home Wi-Fi or the radio's temporary setup network.

## Open the page

On your home network, open `http://avela.local/` when the radio uses its default name. If its name has changed, use
that name instead. You can also enter `http://` followed by the address shown on Info, for example
`http://192.168.1.23/`. That number is an example; use your radio's address.

For initial connection without home Wi-Fi, follow
[Wi-Fi setup](../../../avela_network/docs/user/WIFI.md#connect-for-the-first-time).

On your home network, enter the user name `avela-admin` and the page password in the browser's login prompt. Use
the password you set on Access; your browser can remember the login. No page password is set at first. If the page
says No page password yet, [set one from the setup network or over USB](#set-or-reset-the-page-password).

The page uses unencrypted HTTP. Use it on a trusted network: someone recording its traffic can read the login.
[Firmware updates](../../../avela_firmware_update/docs/user/UPDATE.md) also require opening the update screen on
the radio.

## Choose a tab

| Tab | What you can do |
|---|---|
| Player | [Choose a station, set volume, play or stop](../../../avela_radio/docs/user/RADIO.md#play-from-the-setup-page) |
| Wi-Fi | [Save network credentials or change the radio name](../../../avela_network/docs/user/WIFI.md) |
| Access | [Set or replace the page password](#set-or-reset-the-page-password) |
| Stations | [Add, edit or remove station entries](../../../avela_radio/docs/user/RADIO.md#add-a-station) |
| Weather, when available | [Choose a place and measurement units](../../../avela_weather/docs/user/WEATHER.md) |
| Clock | [Choose the time zone](../../../avela_clock/docs/user/CLOCK.md) |
| Board | [Change brightness or restart](../../../avela_boards/docs/user/DEVICE.md) |

On the home network, opening the page normally selects Player. On the temporary setup network, it selects Wi-Fi. A
tab shows the controls for that section rather than one combined form for all settings.

## Set or reset the page password

If no password is set, [join the radio's setup network](../../../avela_network/docs/user/WIFI.md#connect-for-the-first-time)
to open the page. Set the page password before saving your home Wi-Fi, because saving that network closes setup.

1. Open the Access tab.
2. Enter a New password with 8–32 printable ASCII characters: letters, digits, spaces and keyboard symbols.
3. Press Save and read the result. Use the new password for later home-network logins, with user name
   `avela-admin`.

Access says whether a password is set; it never displays the saved password. Use a password you do not use
elsewhere. The radio stores a hash, but someone with a copy of its flash can still try password guesses.

If you forget the password, open the setup network from the radio and set a new one on Access, or
[replace it over USB](USB.md#set-the-page-password-over-usb). The setup network normally needs no page login:
it opens only from the radio's controls, so you can recover access at the device.

When the home network's address range overlaps the setup network, for example a home network on `192.168.4.x`,
the page requires its login on both networks. Reset a forgotten password over USB in that case.

## Save a change

Use the Save or action button beside the values you edited, and read the result above the form. Wi-Fi has separate
network and radio-name forms; Stations has a form for each entry. A Save button applies its own form only.

A validation error means a value needs correcting; the message identifies what was refused. A storage error means
the radio could not save it. Reload the page to check saved values before retrying. The radio may briefly report
that a playback command has been sent but is not yet carried out; the device screen shows its eventual state.

The page is not a continuously updating dashboard. Reload it to refresh values changed from the knob, another
browser or USB. The Wi-Fi password field's
[blank-value rule](../../../avela_network/docs/user/WIFI.md#change-the-home-network) is important when saving that
form.

## If the page does not open

On the home network, check Info for an address and confirm your phone or computer is on the same network. If the
`.local` name does not resolve, use the numeric address. Guest-network isolation, a VPN or a router rule can prevent
devices reaching one another.

Type the full `http://` address. The radio serves HTTP rather than HTTPS. If you just changed networks or restarted
after changing its name, read the current address from Info again.

If the browser keeps asking for a login, check the user name is exactly `avela-admin` and use the page password,
which is separate from the Wi-Fi passwords. A No page password yet or Page password refusal needs the
[password setup or recovery procedure](#set-or-reset-the-page-password).

On the temporary network, keep Wi-Fi setup open on the radio, stay connected despite the no-internet message, and
use the address shown there. Saving credentials or leaving that screen closes the temporary network.

For configuration without Wi-Fi, use [USB configuration](USB.md).
