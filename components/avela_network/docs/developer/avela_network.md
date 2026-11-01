# `avela_network`: Wi-Fi and the setup network

The network component connects to a saved home network, provides an explicitly requested setup network, and reports
addresses and signal state. It also runs captive DNS and starts mDNS/SNTP integration. Its Wi-Fi setup screen uses
the [network skin contract](SKIN.md).

## Home-network state machine

| Condition | Behavior |
|---|---|
| No credentials saved | Report not_configured; do not open setup automatically |
| Credentials available | Attempt a join immediately |
| A connected network is lost | Start the first rejoin immediately |
| A join attempt fails or times out | Wait 30 seconds before the next attempt |
| Joined | Report connected and current address/signal |
| Setup network active | Do not start home-network join attempts |

Retries continue so a router that starts later after a power failure can be joined without restarting the radio.
Join attempts scan channels and can disconnect a phone from the setup AP, so they are suppressed while that AP is
active.

ESP event handlers publish atomics inside `EspWifiService`. Its `update()` transfers state to `NetworkModel` on the
main loop. Wi-Fi power saving is disabled for continuous streaming; the
[radio reference](../../../avela_radio/docs/developer/avela_radio.md#buffering-and-recovery) explains the playback
reason.

## Setup network

`SetupNetworkRequest` opens or closes the device's setup network. The application requests opening when Wi-Fi setup
is shown and closing when it is left. It does not open merely because the home network is absent or unreachable.

Menuconfig's Avela network settings supply the setup name and password. Defaults are `Avela-setup` and `avela_657`,
using WPA2 with a fixed default password. The password accepts 8–63 characters. These AP defaults are distinct from
the user's stored home-network credentials.

The radio uses APSTA mode, so an existing home connection can remain joined while setup is open. `CaptiveDns`
answers names with 192.168.4.1, and the web server redirects unknown addresses to the setup page while the AP is
active. The [setup reference](../../../avela_setup/docs/developer/avela_setup.md#http-page) owns page routing.

Any save of `WifiCredentials` after initialization closes the setup network and follows the saved credentials, even
when the values are unchanged. That behavior follows the record revision, whether saved through the page or console.
If Wi-Fi setup is shown when the AP closes, the application returns to Info.

## Stored credentials and hostname

`WifiCredentials` is a `Settings` record containing the home SSID and password. Home credentials are entered through
the setup frontends, not menuconfig or git. The driver's own persistent copy is disabled with `WIFI_STORAGE_RAM`.

`WifiSection` uses separate web forms for network credentials and host name. A partial submission keeps omitted
fields. SSIDs have a 32-byte text capacity; an empty password allows an open network, otherwise the WPA2 password
range is 8–63 characters.

`HostName` is a separate record. The saved name overrides menuconfig's `AVELA_HOST_NAME`, default `avela`.
`EspWifiService` applies it once in `init()` to DHCP and mDNS, so a changed name takes effect after restart.
`NetworkModel::m_host_name` reports the name in force, rather than an unapplied saved value.

## Discovery and time

mDNS begins during initialization and advertises the host as `<name>.local`. After joining, SNTP uses pool.ntp.org
to set the system clock. ESP-IDF sets time before the callback marks `SystemClock` synchronized. The
[clock reference](../../../avela_clock/docs/developer/avela_clock.md#clock) owns the synchronization contract.

The host name is supplied as a value to Info, Weather's configuration hint, and the Firmware update screen. Feature
skins do not query network services themselves.
