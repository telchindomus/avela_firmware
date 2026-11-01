# `avela_status_tray`: the shared header

`StatusTrayView` feeds a header skin with local time, requested/actual playback, volume, and Wi-Fi status. It is a
header-only view used by the application's `Ui`, outside the active screen. It has no controller and accepts no
input itself.

The view reads `SystemClock`, `RadioModel`, and `NetworkModel`. It calls the header's setters every frame, allowing
the skin to update changed values and time-based animation steps. A skin does not create a task or timer to animate
the header.

Playback controls remain in each screen's controller through `PlaybackControls`. Their
[application mapping](../../../../docs/developer/FEATURES.md#commands-on-each-screen) determines where they are
available. The [header skin contract](SKIN.md) defines displayed units, ranges, and synchronization behavior.
