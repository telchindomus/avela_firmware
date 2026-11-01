# `avela_boards`: board ownership and wiring

A board owns its physical parts and configures them with compile-time descriptions: pins, timings, rotation, screen
geometry, and hardware settings. Generic implementations come from
[avela_hal](../../../avela_hal/docs/developer/avela_hal.md). The board component also provides its persistent
brightness and restart setup section.

## What a board is

| Level | Owns or requires |
|---|---|
| HAL part | A generic driver or input implementation configured by parameters |
| Module | Parts on the purchased PCB: `Cyd_1_9` owns the display and BOOT button |
| Avela board | The module plus wired parts and their roles: `AvelaPiccolo` adds the encoder and amplifier |
| `BoardHardwareConcept` | Screen description, `init()`, `display()`, and `poll_inputs()` |
| `AvelaBoardConcept` | `BoardHardwareConcept` plus `knob()`, `knob_button()`, `back_button()`, and `audio_output()` |

`Cyd_1_9` does not know whether BOOT means Back. Its protected constructor accepts the button timing, and it exposes
the BOOT input to the assembled board through a protected accessor. `AvelaPiccolo` assigns Back timing and aliases
`back_button()` to the module's `boot_button()`. Naming a hardware role does not assign the application command.

The board owns its `InputPoller`. `poll_inputs(now_ms)` forwards to it and returns input events as a span, valid
until the next poll. Each level keeps its own input list; the assembled board includes the module's button and
replaces `poll_inputs()` without virtual dispatch. The forwarding method stays one line.

Input events carry senders so `Application` can compare them with `board.knob()`, `board.knob_button()`, and
`board.back_button()`. Only `Application` assigns their commands; the
[application mapping](../../../../docs/developer/FEATURES.md#input-mapping) is independent of event detection.

Every module and board checks its own concept with `static_assert`. `Application` checks `AvelaBoardConcept` for the
selected Board type. A module or another device without the extra controls/audio can satisfy `BoardHardwareConcept`
alone.

## Build selection and configuration

Menuconfig's board choice selects `CONFIG_AVELA_BOARD_HEADER` and a header that names `using Board = ...;`.
`Application` holds that Board without naming a concrete board or part. Its configured rotation determines
`Board::screen`; the application checks the skin's screen dimensions against it.

The hardware implementation depends on HAL. The component also requires `Settings` and Setup for its board section,
and LovyanGFX for the display.

Flash layout is project-wide. `sdkconfig.defaults` and `partitions.csv` describe the piccolo's 4 MB flash. A board
with different flash must supply compatible defaults and, when the slots do not fit, a different partition table.
[Build and flash](../../../../docs/developer/BUILD.md) owns the USB and network update procedures.

## Board setup settings

`BoardSection` exposes screen brightness and restart through the shared web/console section interface. BoardSettings
stores brightness in the `board` record, version 1. The accepted range is 10–100 percent, with a default of 80
percent. `Application` follows saved settings on the main loop and converts the percentage to a backlight level from
0 to 255.

Restart is an action, not a persisted model field. Use the section's `restart=yes` action; brightness changes and
restart validation remain owned by `BoardSection`. The
[setup reference](../../../avela_setup/docs/developer/avela_setup.md) explains frontend execution.

## Supported boards

### avela piccolo (`AvelaPiccolo : Cyd_1_9`)

The module is a CYD 1.9-inch device: ESP32-WROOM, 4 MB flash, no PSRAM, ST7789 panel with physical dimensions
170×320 pixels, and no touch. Avela uses the rotated 320×170 landscape screen. `include/avela_boards/cyd_1_9.hpp`
defines display pins and bus parameters.

| Connection | Wiring |
|---|---|
| Back | Module BOOT button, GPIO0 |
| Encoder | S1 to GPIO22, S2 to GPIO23, KEY to GPIO4 |
| Amplifier I2S | DIN to GPIO27, BCLK to GPIO26, LRC to GPIO25 |
| Amplifier power | Vin to 5 V |
| Amplifier mode | SD open: left and right mixed |
| Amplifier gain | GAIN open: 9 dB; GND: 12 dB; 100 kOhm to GND: 15 dB |

GPIO0 is a strapping pin. Holding BOOT during reset starts the ESP32 flasher instead of the application.

The EC11 module's power pin must connect to 3V3. If left floating, encoder turns can appear as KEY presses.

Connect the speaker only between the MAX98357A's two output pads. Its output is bridged; neither speaker lead goes
to ground.
