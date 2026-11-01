# `avela_clock`: local time and synchronization

`SystemClock` provides the current local time and whether the system clock has been synchronized. It is an
all-static device-wide class so ESP-IDF's C callback and the application reach the same synchronization flag.

## Clock

The system clock starts from 1 January 1970 at power-on and ticks until it is set. ESP-IDF's SNTP sets it after
Wi-Fi joins. The callback calls `mark_synchronised()`; it must not set the time a second time.

`now()` returns local calendar/time fields and `m_synchronised` together in `LocalTime`, using localtime_r. A reader
such as the shared header reads time directly. There is no published time stream or second tick queue.

Before synchronization, the header shows `--:--` and no date. Its view calls the skin each frame; the displayed
minute normally changes once per minute. Rendering behavior belongs to the
[header skin contract](../../../avela_status_tray/docs/developer/SKIN.md).

## Timezone settings

`init()` applies the timezone after `Settings` initialization. A saved `ClockSettings` record overrides menuconfig's
`AVELA_TIME_ZONE`. The default POSIX timezone is `CET-1CEST,M3.5.0,M10.5.0/3`.

`ClockSection` exposes the named zones in `include/avela_clock/model/time_zones.hpp` on the setup page, while the
console can supply a POSIX TZ string. `SystemClock::update()` applies newly saved settings on the main loop.
`settings()` returns the effective saved-or-default timezone.

## Additional time sources

There is no general clock setter in the current interface. Adding manual time, an RTC, or another source requires an
explicit operation that sets system time and marks synchronization. That operation must be used by the new source,
not by the SNTP callback after ESP-IDF has already set the clock.
