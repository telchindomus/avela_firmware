# `avela_system_info`: device information

`EspSystemInfoService` reports internal memory, firmware version, and uptime on the main loop. `SystemInfoModel`
contains reported values only. Network facts come from `NetworkModel`; restart and brightness belong to the board
component.

## Info screen

The view passes network facts and system measurements to `InfoSkin`. The screen's controls and navigation are
defined by the [application reference](../../../../docs/developer/FEATURES.md#commands-on-each-screen). The
[skin contract](SKIN.md) defines required values and rows.

## Memory and stack diagnostics

The service reads free, minimum-free-since-start, and total internal heap with `MALLOC_CAP_INTERNAL`. It converts
bytes to its reported kilobyte values by dividing by 1024. Uptime is reported in `std::chrono::seconds`; the running
version comes from the app description.

`Application` provides `TaskStack` entries with declared task sizes. Once per minute, the `system` log reports peak
stack use from the high-water mark:

```text
stack used (bytes): main 2504/8192 stream 3180/8192 ...
```

Each pair is used/declared bytes, where used equals declared size minus the lowest remaining stack reported by
FreeRTOS. Missing task handles are skipped.

The main task's configured default is 8192 bytes in sdkconfig.defaults. `Application`-owned worker sizes are
constants of their owners; framework task sizes come from ESP-IDF settings. The canvas buffers alone exceed that
main-stack default, which is why long-lived application storage is kept in the static `Application`.

`HeapWatch` in HAL measures before, lowest, and after heap around stream connection and Weather requests. A log line
can look like:

```text
stream: connected in 820 ms: heap 61 KB before, 38 KB lowest, 52 KB after
```

These are illustrative log values, not guaranteed budgets. Wi-Fi, HTTP, MP3 decoding, and HTTPS/TLS keep
library-managed heap resources. Check the Info screen and logs on the board after memory changes. The mandatory
stack-sizing and exercised-path policy lives in the
[coding standard](../../../../docs/developer/CODING_STANDARD.md#stack-sizing-and-memory-checks).
