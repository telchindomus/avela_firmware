# `avela_hal`: hardware parts and input events

The HAL supplies generic ESP32 hardware parts configured by a board. `GpioButton`, `PcntEncoder`, `St7789Display`,
and `I2sOutput` touch hardware; `ButtonStateMachine`, `DetentCounter`, and `InputPoller` provide platform-free
logic. The component knows no specific board and assigns no application meaning to an input event.

Board ownership and wiring are described in [Boards](../../../avela_boards/docs/developer/avela_boards.md). The
[coding standard](../../../../docs/developer/CODING_STANDARD.md#4-polymorphism-compile-time-only) defines
concept-based interfaces.

## Words (use exactly these)

| Word | Meaning |
|---|---|
| Input | One operated physical input, such as encoder rotation or a push switch; its class satisfies `InputConcept` |
| Input event | What an input did, with its sender; `InputEvent` is the variant of `Turn`, `Click`, `DoubleClick`, and `LongPress` |
| Sender | The input object's identity as `const Encoder*` or `const Button*` |

Do not call an input event a gesture. Distinguish the hardware parts from the platform-free logic rather than
describing every class merely as HAL.

## Inputs

Each input provides `poll(now_ms)`, returning an optional event. The encoder takes the same time argument as
buttons, even though it does not need it. Concrete inputs are checked against `InputConcept`.

```text
PCNT counts -> DetentCounter -> Turn
GPIO level  -> ButtonStateMachine -> Click / DoubleClick / LongPress
                                     |
                              InputPoller event span
```

Button and Encoder are empty, non-copyable bases used for identity. A different hardware implementation can preserve
that sender role without virtual functions. Events carry the originating object's address; a receiver compares it
with the board's accessor, such as `board.back_button()`. It must not infer identity from polling order.

`InputPoller` holds references to the board-owned inputs in a tuple. It calls each input directly, in order, and
returns a span valid until the next poll. Storage has one event slot per input; each input returns at most one event
in a poll. Encoder detents are combined into one `Turn`, and button debounce prevents multiple completed events
inside the application's input-poll interval.

## Detecting rotation and button presses

`PcntEncoder` uses ESP32 PCNT counting with a glitch filter and x4 quadrature. Each poll reads the difference in
counts; `DetentCounter` emits a detent for four counts and retains an incomplete detent for later. Slow turns
therefore accumulate correctly without a separate input task or GPIO interrupt handler.

`GpioButton` reads the pulled-up pin, where pressed is zero, and passes the level and time to `ButtonStateMachine`.
The machine applies the selected `ButtonTiming`.

| Timing/event | Encoder button default | Back button |
|---|---|---|
| Debounce | 20 ms | 20 ms |
| `Click` | 300 ms after release, unless a second press begins in that window | On release, regardless of hold duration |
| Double click | On the second release | Disabled |
| Long press | Once after 600 ms while held | Disabled |

The release after an encoder long press emits no click. With double-click handling disabled, no extra post-release
wait is needed. Setting long_press_ms or double_click_ms to zero disables that event in `ButtonTiming`. These
settings are defined in `include/avela_hal/button/button_timing.hpp`.

The application polling interval belongs to [Controls](../../../../docs/developer/FEATURES.md#main-loop). Consumers
receive completed input events, not access to each input's pressed state or partial counter state.

## Display and audio parts

`St7789Display` wraps the panel configuration and LovyanGFX device used by the canvas. Board descriptions provide
pins, bus parameters, panel geometry, rotation, and backlight configuration. The drawing interface is the
[UI canvas](../../../avela_ui/docs/developer/avela_ui.md#drawing), which receives the board's display rather than a
board-specific type.

`I2sOutput` supplies sample-rate-paced audio writes and an explicit stop operation. The
[radio reference](../../../avela_radio/docs/developer/avela_radio.md#buffering-and-recovery) defines when playback
stops and restarts those clocks; the board reference owns amplifier wiring. HAL memory helpers provide `HeapWatch`
measurements used by services.

Public headers are grouped by topic under `include/avela_hal/`: input/events, button, encoder, display, audio,
memory, and board. The platform-free input state machines are tested in `tests/avela_hal_test/`.
