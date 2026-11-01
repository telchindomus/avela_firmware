# Writing an Avela skin

A skin controls the palette, fonts, images, layout, wording, and motion. Components and the application control
behavior. Write a skin as its own ESP-IDF component so it can be developed and distributed without modifying
firmware behavior.

Read [Skin architecture](SKIN_ARCHITECTURE.md) for the integration/version rules. The
[shared contract](../../components/avela_ui/docs/developer/SKIN.md),
[header contract](../../components/avela_status_tray/docs/developer/SKIN.md), and each component's `SKIN.md` specify
what to implement.

## What a skin is

```text
avela_skin_example/
  CMakeLists.txt
  Kconfig
  idf_component.yml
  include/avela_skin_example/skin.hpp
  assets/
```

CMake declares source files and unconditional dependencies on `avela_ui` and components whose page skins are
implemented. Optional page sources are added under their feature switch. Kconfig adds the skin's menu choice and
defaults. The manifest describes its version/licence, and assets include their licences.

`skin.hpp` supplies the Skin alias, page-skin types, Fonts, Surface, and supported screen. Main constructs the
surface from its canvas and fonts, then constructs each page skin from that surface.

## Your entry in menuconfig

Follow [rule 3](SKIN_ARCHITECTURE.md#rules) for the Kconfig entry. Choose a unique `AVELA_SKIN_<NAME>` symbol, set
`AVELA_SKIN_COMPONENT` to the component folder name, and state the firmware API version implemented in
`AVELA_SKIN_API_VERSION`. Use a readable menu label that identifies size and appearance.

Copying an official skin supplies a starting API version matching that firmware. After copying, the version is yours
to maintain under [Compatibility](#compatibility).

## What you implement

Implement a page skin for every enabled screen required by the firmware, plus `HeaderSkin` and `TilesSkin`.
Component references list exact setters and value ranges. Setters accept concrete values rather than mutable models;
a compound value such as `WeatherReport` is part of the skin API.

Before `draw()`, setters keep values. `draw()` renders the initial area. Later setters redraw changed visible
output, including a changed animation step. Keep time-dependent state stable throughout a band render. Draw only
when called on the firmware task; do not start a drawing task or timer, because the display, canvas buffers, and
LovyanGFX are not thread-safe.

For animation, use the repeated setter calls and `std::chrono::steady_clock`. The header contract demonstrates Wi-Fi
and playback motion. The [UI drawing reference](../../components/avela_ui/docs/developer/avela_ui.md#drawing)
explains band callbacks and storage.

Support the board's post-rotation screen size. Main checks `Skin::screen` against `Board::screen`. A missing
per-screen layout should fail clearly in the skin's `screen.hpp`. Guard optional sources and aliases with
`CONFIG_AVELA_WEATHER` while keeping the component `REQUIRES` unconditional.

## Developing

1. Check out the firmware release whose API you intend to implement. Copy an official skin to a sibling folder and
   give it a distinct component name. For example:

   ```text
   workspace/
     avela_firmware/
       main/
     avela_skin_example/
   ```

2. Rename the component's include prefix, internal skin namespace, exported type/alias, CMake asset references, and
   Kconfig selection/defaults consistently. Retain the public `avela::Skin` alias and the required page-skin member
   names. Keep the component contracts unchanged.

3. Create `main/idf_component.yml` in the firmware checkout if it does not exist. Merge the dependency into an
   existing manifest rather than replacing its other entries:

   ```yaml
   dependencies:
     avela_skin_example:
       path: ../../avela_skin_example
   ```

   Local paths resolve from the manifest's directory. This example traverses out of main, then out of the firmware
   folder, to the sibling skin.

4. Run `idf.py menuconfig`, choose your skin under Avela skin, then run `idf.py build`. Check the version and
   screen-size diagnostics before changing interface code.

5. Flash and exercise every screen, the shared header, navigation, and animations. Test optional Weather both
   enabled and disabled. Build commands and required checks are in [Build and flash](BUILD.md).

## Assets

Each skin owns its palette and font roles. Drawing uses roles such as title or caption rather than selecting a font
weight/size at a call site. Layout structs live in the skin; per-screen folders supply designated initializer
values. Add a supported screen through board Kconfig, the generator's `SKINS` mapping, and the skin's layouts.

The official asset generator reads each skin's `assets/fonts/Manrope-wght.ttf` and `assets/logo/avela_symbol.svg`.
Manrope uses its accompanying OFL licence. `FONT_ROLES` must match `fonts.hpp`, `assets.hpp`, and the CMake
embedded-file list. Roles include `display`, `headline`, `title`, `heading`, `figure`, `subtitle`, `list_item`,
`body`, `body_strong`, `caption`, and `badge`.

Run regeneration explicitly when those sources or role settings change:

```sh
python3 -m pip install pillow resvg-py
python3 tools/generate_assets.py
```

The generator writes one VLW font per role/screen and Avela logo data to each skin's `assets/generated/<screen>/`
folder. The logo output is `avela_logo.argb`, raw ARGB8888, embedded with `EMBED_FILES` and drawn from flash. The
current 22×20 symbol uses sun, wave, and bar paths without a background; the generator strengthens the wave at this
small size.

Commit generated outputs. The firmware build consumes them directly and does not run the asset generator. Outside
skins may use their own asset workflow, while preserving the public font/surface contract.

## Distributing

Publish the skin repository with tagged releases. A firmware user can add it to main's manifest by tag:

```yaml
dependencies:
  avela_skin_example:
    git: https://github.com/<you>/avela-skin-example.git
    version: v1.0.0
```

It may also be published under a namespace in the ESP Component Registry. The downloaded component is configured
with the firmware and appears in the skin choice.

The component manager caches git repositories. If a newly created tag is not found, remove only that repository's
cache entry under `~/.cache/Espressif/ComponentManager/` and retry. This is a local dependency-cache issue, not a
skin API version change.

## Compatibility

`AVELA_SKIN_API_VERSION` identifies the firmware API your page skins implement. Your skin's own manifest/git version
remains independent. The firmware's `supported_skin_api_versions` constraint decides whether that implementation is
accepted; a newer-than-firmware API is always refused.

When firmware requires additions or changes, follow [Skin changes](SKIN_CHANGES.md), implement the listed changes,
and update your declared API version. Removals do not advance the lower bound; delete removed page code and
`REQUIRES` entries when configuration or compilation identifies them. Pages for disabled components remain excluded
by their switches.

The complete version policy and supported comparison operators are defined once in
[Versions and skin compatibility](SKIN_ARCHITECTURE.md#versions-and-skin-compatibility).

## Licence

The firmware is MIT licensed. Your skin may choose its own licence, including a paid licence. Fonts and images keep
their own licences; ship those with the assets.

## Official skins

TelchinDomus maintains [Piccolo](../../components/avela_skin_piccolo/README.md), the deep teal default, and
[Silk Taupe](../../components/avela_skin_piccolo_silk_taupe/README.md), using taupe, espresso, and champagne gold.
Both target the piccolo's landscape screen and are independent components. Only officially supported skins are
listed by TelchinDomus.

Use either as a complete example. Its README links to the palette, layouts, and asset workflow that belong to that
skin.
