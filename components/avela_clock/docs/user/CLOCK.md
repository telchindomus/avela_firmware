# Clock

The status bar shows the local time and date. Avela sets its clock from an internet time server after joining Wi-Fi.
You choose the time zone; you do not need to enter the date and time manually.

## Set the time zone

1. Open the [setup page](../../../avela_setup/docs/user/SETUP.md) and choose Clock.
2. Select the time zone for your location. Some choices list several cities that share the same rules.
3. Press Save. The local display changes without a restart.

The selected zone includes its daylight-saving rule, so seasonal changes happen automatically for zones that use
them. The default firmware zone is Central European Time, with its daylight-saving rule.

If your zone is not listed, the
[USB configuration tool](../../../avela_setup/docs/user/USB.md#change-a-setting-from-the-command-line) can set the
`clock` section's `time_zone` field to a POSIX TZ string. For example, this selects UTC:

```sh
avela-tuner --port <PORT> set clock time_zone=UTC0
```

Custom strings must contain 1–63 printable ASCII characters. Enter a valid rule for the desired location; the radio
checks length and characters, not whether the rule describes the intended place.

## Clock problems

`--:--` means the clock has not synchronized since startup. Check that the radio has joined a network with internet
access and can reach its time service. It does not retain a synchronized clock through loss of power.

If minutes are correct but the hour is wrong, check the selected time zone. For a seasonal one-hour error, check its
daylight-saving rule. Restarting does not correct a wrong saved zone.

There is no manual date/time control in the setup page. Setting a zone changes the local representation; it does not
supply the missing time synchronization.
