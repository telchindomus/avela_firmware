# `avela_radio`: stations and playback

The radio component owns station selection, saved playback, stream reception, MP3 decoding, volume, and spectrum
data. The player and Stations screens expose that state; the setup page and console use the same service request
interface as the encoder.

`Application` controls are described in [Controls](../../../../docs/developer/FEATURES.md#controls). The
[skin contract](SKIN.md) defines the values a radio page skin receives.

## Stations and saved playback

Stations occupy 16 numbered `Settings` slots. `StationRecord` uses keys `station1` through `station16`; an empty URL
marks a free slot. Names and genres each hold up to 32 bytes of text, and a URL holds up to 200 bytes. Capacities
include one additional byte for termination in the record's FixedStrings.

The model's `StationList` keeps names, genres, monograms, and slot identities in RAM, rather than retaining every
URL. It uses about 2 KB. The service reads a URL from its record when tuning. A monogram uses the first letters of
up to three words; a single-word name uses its first two letters, with ASCII letters uppercased.

If no slot contains a station at startup, `RadioStorage` stores the default stations. It reloads the list when the
`StationRecord` revision changes. The current station is preserved by slot if it still exists; otherwise selection
falls back to the first entry. A screen's selection is clamped when the list shrinks. If no stored station is
readable, the model uses an unsaved default list.

`RadioStorage` initializes after `Settings` and updates before the streaming service. It restores `PlaybackMemory`
at startup and saves the station slot, volume, and requested playing state 10 seconds after the last change. A
stopped radio remains stopped after restart.

## Playback states and requests

`PlayerRequest` carries a station slot, requested playing state or toggle, and volume or a relative change. The
streaming service takes requests on the main loop. `carry_out(request, model)` handles the requested fields, and
`PlayerStatus` reports the resulting state to frontend clients. Initialization and station-list reload also restore
or reconcile model state through `RadioStorage`.

| State | Meaning |
|---|---|
| `stopped` | Playback is paused, or the home network is unavailable |
| `connecting` | A stream is being opened or buffered |
| `playing` | Audio output is active |
| `failed` | Reception failed and recovery is pending |

Requested playing state and actual playback state are distinct. The header and player skin receive both so they can
show waiting or failure without losing the request to play.

## Receiving a stream

`StreamingRadioService` owns a receiver task and a player task. Its `update()` runs on the main loop and transfers
requests and reports between the model and those workers.

```text
HTTP/HTTPS -> ICY metadata removal -> encoded-audio queue -> MP3 decoder
                                                              |
                                                    spectrum measurement
                                                              |
                                                         volume gain
                                                              |
                                                        I2S output
```

`StreamReceiver` uses HTTP or HTTPS. HTTPS checks the server certificate with the ESP-IDF certificate bundle.
Requests send `Icy-MetaData: 1` and a User-Agent of `Avela/<version>`; the latter is required by some station
servers. The receiver follows up to five redirects and accepts a response with `Content-Type: audio/mpeg`.
`IcyMetadataReader` separates titles from audio bytes.

Audio is queued in 1024-byte EncodedAudioChunks. Each connection has an atomic session number, copied into its
chunks. The player skips chunks from an obsolete session after a stop or retune.

## Buffering and recovery

| Condition | Behavior |
|---|---|
| Encoded-audio buffering | Queue of 12 chunks, approximately 0.75 seconds at 128 kbit/s |
| New session | Wait for half the queue to fill, for at most 3 seconds |
| Stop or retune during that wait | Recheck the session and discard obsolete audio |
| Connection failure | Retry after 5 seconds |
| End of stream | Reconnect immediately |
| Open connection with no data | Fail after 3 consecutive read timeouts, 5 seconds each |
| No player chunk for 200 ms | Report no sound and stop the I2S clocks |

When I2S clocks stop, the MAX98357A amplifier enters shutdown, avoiding idle hiss. The next output write restarts
the clocks. DMA automatically supplies silence until output is stopped. The timeout also stops clocks left by a
previous session if the new session never produces audio.

Wi-Fi power saving is disabled in `EspWifiService` using `WIFI_PS_NONE`. Modem sleep can deliver stream data in
bursts and interrupt audio even while the UI keeps updating.

## Decoding, volume, and spectrum

`Mp3Decoder` accepts bytes split at arbitrary boundaries and spreads mono samples to stereo. `I2sOutput` writes are
paced by the sample rate; the service changes that rate to match decoded frames. Board wiring is defined in the
[board reference](../../../avela_boards/docs/developer/avela_boards.md).

Volume gain is linear in the requested knob position: midpoint is approximately −6 dB, one quarter is approximately
−12 dB, and zero is silence. An atomic carries the current gain to the player task.

`SpectrumAnalyzer` measures before volume is applied, so changing the knob does not change the measured bars. It
uses a 512-point FFT per decoded frame, a Hann window, and 32 logarithmically spaced bands from 50 Hz to 16 kHz,
mapped over −60 to 0 dBFS. The skin chooses how those values look and move.

## Tasks and diagnostics

| Worker | Task name | Core | Priority | Static stack |
|---|---|---|---|---|
| `StreamReceiver` | `stream` | 0 | 4 | 8192 bytes |
| `Mp3Player` | `player` | 1 | 5 | 4096 bytes |

The workers exchange requests, audio, and reports through FreeRTOS queues in static storage. The implementation
names are `m_tune_requests_`, `m_encoded_audio_`, `m_latest_reception_`, and `m_latest_playback_`. Tune requests and
latest reports use one-item overwritten queues. Their worker reads consume the latest item. The external
`StatusQueue` interface provides a nondestructive read, as described in
[Messaging](../../../avela_messaging/docs/developer/avela_messaging.md).

While tuned, the `radio` log reports free heap, lowest free heap, largest block, queue fill, player dropouts, and
remaining worker stacks every 10 seconds. A dropout means sound stopped while the station was still requested.
General heap/stack measurement policy is in the
[coding standard](../../../../docs/developer/CODING_STANDARD.md#3-memory-no-heap-in-our-code).

## Setup integration

`StationsSection` validates all changed slots before saving and keeps at least one station. Field names are
`<slot>.name`, `<slot>.genre`, and `<slot>.url`. The web page writes one form per used slot and one for a free slot,
using `WritesOwnFormsConcept`, so the station set need not fit one request. Buffer limits belong to the
[setup reference](../../../avela_setup/docs/developer/avela_setup.md#limits-and-execution).

`PlayerSection` accepts `station`, `playing` as yes/no, and `volume`. It reads the published status and briefly
waits for its submitted request to be reflected there; the setup reference explains frontend task boundaries.

### Default station checks

The six shipped records provide these stream examples. Recheck the live behavior when using them to test a change,
because providers can change their endpoints or stream format.

| `Station` | What to check |
|---|---|
| SomaFM Lush | User-Agent `Avela/<version>` is required by the configured service |
| Radio Paradise | Plain HTTP, 128 kbit/s MP3 |
| FIP Jazz | Playback without stream titles |
| Radio Swiss Jazz | 48 kHz audio after two redirects |
| Groove Salad | User-Agent `Avela/<version>` is required by the configured service |
| Lush HTTPS | TLS connection and its heap cost, compared with the HTTP Lush entry |

The User-Agent behavior of Lush and Groove Salad is the reason the receiver sends that header. Live observations are
test cases to verify, rather than permanent promises about external services.

## Limitations and tests

Only MP3 streams are supported. AAC and playlist files such as .pls and .m3u are not supported.

In the official skins, a remote edit can leave the open Stations screen visually stale when the list length and
selection do not change. Moving the selection or reopening the screen causes a redraw. The model reload and
selection bounds remain current; this is a display refresh limitation.

The component uses Espressif's binary esp_audio_codec library and esp-dsp. The codec carries Espressif's modified
MIT terms for Espressif chips; esp-dsp uses Apache-2.0. Retain the dependencies' licence terms when distributing the
firmware.

`tests/avela_radio_test/` covers the ICY reader, volume, spectrum bands, station list and fields, and player screen.
