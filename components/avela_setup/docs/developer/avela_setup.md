# `avela_setup`: web and serial settings frontends

Setup exposes component sections through a web page and a serial console. Both frontends call the same
validation/apply operation and use the same field descriptions and result messages. Components own their settings;
this component owns transport, forms, parsing, and frontend execution.

## Defining a section

`Application` supplies the same section list to `SetupServer` and `SetupConsole`. A section satisfies
`SetupSectionConcept`:

| Member | Purpose |
|---|---|
| id | Section identifier, using letters and underscores |
| title | Human-readable heading |
| fields | `SetupField` entries with name, description, and secret flag |
| `write_form(HtmlWriter&)` | Write the web fields |
| `apply(FormFields)` | Validate submitted values and return `ApplyResult` |
| `write_values(ValueWriter&)` | Write current values and notes for console readers |

A normal web section receives a surrounding form and Save button. A section that supplies multiple forms uses
`WritesOwnFormsConcept`; Wi-Fi and Stations do this. Use `HtmlWriter`'s text escaping for values rather than
emitting user text as raw HTML.

An omitted field preserves its saved value. A section that updates only part of a record must merge against the
current stored record, using
[Settings::update](../../../avela_settings/docs/developer/avela_settings.md#atomic-partial-updates) when another
task can save that record.

`ApplyResult` reports whether the action was saved, its `ApplyError`, an affected field, and a human-readable
message. Error kinds are none, invalid_value, storage_failed, and unknown_field. The console checks unknown field
names before applying a section.

## HTTP page

`SetupServer` serves port 80 on all interfaces: the home network when connected and the setup network when active.
A GET of `/?tab=<id>` shows only that tab's form; ordinary settings forms need no JavaScript.

No tab or an unknown tab selects the landing section. Main chooses Player on the home network and Wi-Fi while the
setup network is active. POST /save applies the named section and returns the same tab with the result above it.
Unknown addresses redirect to the setup page while the setup network is up.

## Page password

On the home network every page asks for the user name `avela-admin` and the page password through HTTP Basic
authentication. A server can't fill in the browser's prompt, so the prompt's title (the realm, shown by some browsers)
and the refusal page name the user name; a browser's password manager remembers both after the first login. On the setup network no page does: whoever joined it is at the radio, and it is the way back in when the
page password is forgotten.

A connection counts as coming from the setup network only when all three hold (`setup_network_client.hpp`, tested in
`tests/avela_setup_test`):

1. The client's address is in the setup network.
2. It connected to the radio's setup-network address. lwIP accepts a packet for any of the radio's addresses on any
   interface, but the radio answers its setup address only on the setup network.
3. The home network doesn't overlap the setup network. If it does, for example a home network on 192.168.4.x, a
   home client could hold a setup-network address and be answered at the radio's home address. The check then fails
   closed: the page asks for the password on both networks, and USB is the way back in.

`setup_page::allowed()` makes the check, and every handler on the server calls it first, including the additional
routes below. A wrong or missing password gets 401 with a `WWW-Authenticate` header, so the browser asks for it.
Until a password is set, the home network gets 403. Both refusals are small pages in the setup page's style with the
steps to set a password. After a refusal, a handler returns `setup_page::after_refusal()`: for a request with a body,
ESP_FAIL, so the server closes the connection instead of reading the body first. An upload is megabytes, and a slow
client would hold the server's one task. A client still sending sees a broken connection, so `avela-tuner update`
checks the password with a GET of `/update` before it sends the image. The 404 handler redirects to the page only
for a connection from the setup network.

The password is set on the setup network or over USB, both only for someone at the radio. `AccessSection` (tab
Access, console section `access`, field `password`) takes 8 to 32 printable ASCII characters. It is write-only: the
tab and the console say whether one is set, and settings dumps leave it out, so a restore keeps the radio's own.
While none is set, the Wi-Fi tab on the setup network reminds to set one first, because joining the home network
closes the setup network.

`PagePassword` is a `Settings` record that holds only a PBKDF2-HMAC-SHA256 hash (1000 rounds) and its 16-byte random
salt, so the password can't be read from the flash. A copy of the flash still lets someone try guesses against the
hash on a fast computer, so the user docs recommend a password used nowhere else. Every request with a password runs
the hash once; the radio logs how long the first check took.

The page is plain HTTP. The password keeps out other people on the home network, not someone who records its
traffic.

## Additional routes

The server exposes `http_server()` for a component with additional routes.
[Firmware update](../../../avela_firmware_update/docs/developer/avela_firmware_update.md#upload-endpoint) adds its
upload page and raw-image endpoint; its JavaScript upload behavior is separate from ordinary settings forms.

## Human console

`SerialConsole` reads the ESP console UART, UART0 in the current configuration, at 115200 baud. Connect with
`idf.py -p /dev/ttyUSB0 monitor` or a serial terminal such as picocom or screen.

| Command | Result |
|---|---|
| help | List commands, sections, and fields |
| `<section>` | Show current values |
| `<section> name=value ...` | Apply fields through the section |
| restart | Restart the radio |

Quote values containing spaces. Inside quotes, `\"` represents a quote and `\\` a backslash:

```text
weather place="New York" units=imperial
wifi ssid="My \"Home\"" password="example password"
```

`SerialConsole` echoes input, supports backspace, and writes through fwrite rather than printf varargs. Log lines
may appear between replies. `OutputLock` holds stdout for a whole reply so another task's log cannot split it. Do
not log while holding that lock, and do not take another lock whose owner may log while holding it. `Settings` reads
are safe at the configured default log level.

## Machine protocol

Lines beginning with a colon receive one JSON reply. The protocol version is 2, declared by
`setup_console::protocol_version`. Raise it when a command or reply changes.

| Command | Reply contents |
|---|---|
| `:sections` | ok, protocol, and section/field descriptions including secret flags |
| `:get <section> [secrets]` | ok, values, and notes; secret values only when explicitly requested |
| `:set <section> name=value ...` | ok and a message, or error/field/message |
| `:restart` | An ok reply, then restart |

Errors include unknown_command, unknown_section, invalid_arguments, invalid_value, unknown_field, and
storage_failed. Secret fields are hidden in human output and omitted from ordinary machine reads. `write_values()`
supplies every field, empty if unset, so settings dumps can be restored. The host client avela-tuner uses this
protocol.

## Limits and execution

| Resource | Storage or policy |
|---|---|
| HTTP form body | `setup_page::form_capacity`: 1024-byte URL-encoded buffer |
| Console decoded fields | `SetupConsole` form buffer: 512 bytes |
| Serial input line | `SerialConsole::line_capacity`: 384 bytes |
| HTTP server task | `setup_page::server_stack_bytes`: 6144 bytes |
| Serial task | `SerialConsole::stack_bytes`: 4096 bytes |
| Stalled body reception | `max_receive_timeouts` is 3; each timeout uses httpd's default `recv_wait_timeout`, currently 5 seconds |

These are separate buffers, not interchangeable payload limits. URL encoding can use three body bytes for one
encoded input byte, and UTF-8 characters can occupy multiple bytes. Serial bytes past the line buffer capacity are
dropped.

HTTP sections execute on the server task; console sections execute on the serial task. Both hold `ApplyLock` around
`apply()`, allowing only one frontend apply at a time. This keeps a section's validation of current settings and its
save together, such as the rule that at least one station remains.

Sections may use `Settings`, immutable values, or a service's synchronized request/status interface. They must not
read a mutable shared model directly. `PlayerSection` queues its request and waits up to 300 ms for the published
status to match; it reports whether the action was carried out or only sent.

The HTTP server has one task. `setup_page::receive()` stops a body read after the consecutive-timeout limit, so a
client that stops sending cannot block every other page indefinitely. This receive helper is shared by settings
forms and firmware uploads.

## `Application` integration

Main supplies `WifiSection`, `StationsSection`, `PlayerSection`, optional `WeatherSection`, `ClockSection`, and
`BoardSection` in that order. Restore ordering matters: station records must exist before the player selects one.
Their field meanings belong to the respective component references.
