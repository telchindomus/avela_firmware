# Avela firmware: build, flash and checks

Build from the firmware repository root with an activated ESP-IDF environment. The documented SDK is ESP-IDF v5.5.5.
LovyanGFX is pinned to tag 1.2.33 in `components/avela_hal/idf_component.yml`.

## Build and flash

### Build

Activate the installed ESP-IDF environment and build:

```sh
. "$IDF_PATH/export.sh"
idf.py build
```

Use `idf.py menuconfig` to select the board, skin, and optional features. The board's configuration supplies the
screen size. The project flash defaults are in `sdkconfig.defaults`, and the partition layout is in
`partitions.csv`.

### USB update that keeps settings

`<PORT>` is the board's serial port, often `/dev/ttyUSB0` on Linux. Flash with ESP-IDF:

```sh
idf.py -p <PORT> flash
```

The equivalent application update with esptool writes both initial OTA metadata and the image:

```sh
esptool.py --chip esp32 --port <PORT> write_flash \
  0xd000 build/ota_data_initial.bin \
  0x10000 build/avela_firmware.bin
```

Both routes write the app to `ota_0` and select it through otadata. NVS at `0x9000–0xCFFF` is preserved. Writing
only the image at `0x10000` is insufficient: after a network update the bootloader may still select `ota_1`.

### Full image for first installation

A merged image includes the bootloader, partition table, OTA metadata, and application. It erases saved settings
because the merge fills the `0x9000–0xFFFF` gap with `0xFF`, covering NVS, otadata, and phy_init. Preserve settings
beforehand with `avela-tuner dump --secrets` and restore them afterward if required.

Run both commands from the repository root:

```sh
(cd build && esptool.py --chip esp32 merge_bin -o avela_firmware_full.bin @flash_args)
esptool.py --chip esp32 --port <PORT> write_flash 0x0 build/avela_firmware_full.bin
```

The subshell keeps the merge inside `build`, where paths in `flash_args` resolve. `@flash_args` supplies the
generated flash size, mode, and speed. The current table is sized for 4 MB flash, with NVS, otadata, phy_init, and
two app slots, `ota_0` and `ota_1`.

### Network update

The setup page's firmware update and `avela-tuner update` write the inactive app slot and preserve settings. Follow
the [firmware update reference](../../components/avela_firmware_update/docs/developer/avela_firmware_update.md) for
acceptance, failure handling, and rollback confirmation.

Board-specific boot and wiring information is in the
[board reference](../../components/avela_boards/docs/developer/avela_boards.md).

For version changes, remote builds of every official skin, and release artifacts, see
[Releases and versions](RELEASE.md).

## Checks before a commit

Every commit requires a warning-free `idf.py build` and a clean code gate:

```sh
idf.py build
tools/check_code.sh
```

`tools/check_code.sh --fix` formats first, then runs the same checks. The gate covers clang-format, clang-tidy,
native unit tests, skin API consistency, and documentation paths. The mandatory policies are in the
[coding standard](CODING_STANDARD.md).

### Host prerequisites

Install esp-clang through the activated ESP-IDF tools:

```sh
python "$IDF_PATH/tools/idf_tools.py" install esp-clang
```

Native unit tests require `g++`, `ruby`, and `libbsd-dev` on the host. The code gate also needs native
`clang-tidy-19` for host test sources: the ESP cross compiler does not know the host C++ headers. Test projects use
ESP-IDF's Linux target, Catch2, and its FreeRTOS mock.

To build and run every native test project independently of the full gate:

```sh
tools/run_unit_tests.sh
```

### Build coverage

Clang-tidy reads compile commands from `build_clang/`. The gate configures each other skin separately in
`build_clang_<skin>/`, with optional components enabled. If the selected build disables Weather, its excluded
component and skin sources are checked in `build_clang_optional/`, also with optional parts enabled. A production
`.cpp` absent from all applicable source lists fails the gate.

Compile databases are refreshed when their sdkconfig, a `CMakeLists.txt`, or a Kconfig changes. These are
configure-only clang builds; they do not replace the normal GCC build.

GCC warnings cover only the selected skin. Before committing a change to another skin, build it too, using a
separate sdkconfig copy with that skin selected:

```sh
idf.py -B build_<skin> -D SDKCONFIG=<copy-of-sdkconfig> build
```

### Documentation and assets

The documentation and skin checks can also run separately:

```sh
tools/check_docs.sh
tools/check_skin_api.sh
git diff --check
```

The documentation check runs the link-checker regression tests and resolves repository-relative Markdown links and
heading anchors through `tools/check_doc_links.py`. Technical claims and code-cited section names still require
review when moving or rewriting a reference.

Generated font and logo assets are committed. A firmware build does not run the asset generator or need its
Pillow/resvg-py packages; it still uses the Python-based ESP-IDF toolchain. For asset changes, see
[Writing a skin](SKINS.md#assets).
