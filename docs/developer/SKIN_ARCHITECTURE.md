# Skin architecture: components, skins and how they fit together

Components own behavior and declare what must be drawn. A skin implements those drawing interfaces. Main selects a
skin at build time, constructs its page skins, and checks compatibility. [Writing a skin](SKINS.md) gives the
authoring procedure; this reference defines the integration rules.

## Roles

| Owner | Responsibility |
|---|---|
| Feature component | Model, service, screen/controller/view, and page-skin concept |
| Skin component | Palette, fonts, images, layouts, wording, and motion for its page skins |
| Main | Compose models/services/screens, link the chosen skin, and check screen-size compatibility |

A component's `docs/developer/SKIN.md` defines the page-skin contract. The shared Skin, Surface, font, and tile
interfaces belong to the [UI contract](../../components/avela_ui/docs/developer/SKIN.md). The header has its own
[status-tray contract](../../components/avela_status_tray/docs/developer/SKIN.md).

## Rules

1. **Kconfig is the only on/off switch for a component.** An optional component declares `CONFIG_AVELA_<NAME>` in
   its own Kconfig. When disabled, it compiles no sources and skins compile none of its pages. An off boolean option
   has no define in `sdkconfig.h`; optional `ScreenId` entries follow the same switch.

2. **`REQUIRES` is unconditional and does not mean enabled.** ESP-IDF evaluates dependencies during early expansion,
   before `CONFIG_*` values are available. Conditional `REQUIRES` can silently omit dependencies. ESP-IDF also
   configures/builds discovered components that nobody requires; unused linked code can be discarded later. Disable
   through the feature switch rather than deleting a dependency entry.

3. **Choose the skin through menuconfig.** `avela_ui` defines the `AVELA_SKIN` choice and hidden
   `AVELA_SKIN_COMPONENT` and `AVELA_SKIN_API_VERSION` strings. Each skin adds a choice entry and conditional
   defaults for those strings. A repeated choice/symbol definition produces INFO output; this is intentional.

   ```text
   choice AVELA_SKIN
       config AVELA_SKIN_EXAMPLE
           bool "Example skin (320x170)"
   endchoice
   config AVELA_SKIN_COMPONENT
       default "avela_skin_example" if AVELA_SKIN_EXAMPLE
   config AVELA_SKIN_API_VERSION
       default "0.1.0" if AVELA_SKIN_EXAMPLE
   ```

4. **Link the chosen skin into main at the top level.** Main cannot list unknown outside skins in `REQUIRES`. After
   `project()`, the top-level `CMakeLists.txt` selects the library and supplies its entry header:

   ```cmake
   idf_component_get_property(main_library main COMPONENT_LIB)
   idf_component_get_property(skin_library ${CONFIG_AVELA_SKIN_COMPONENT} COMPONENT_LIB)
   target_link_libraries(${main_library} PRIVATE ${skin_library})
   target_compile_definitions(${main_library} PRIVATE AVELA_SKIN_HEADER="${CONFIG_AVELA_SKIN_COMPONENT}/skin.hpp")
   ```

   `main/ui.hpp` uses `#include AVELA_SKIN_HEADER`. Each skin's entry header supplies `using Skin = ...;` in
   namespace avela.

5. **Check compatibility at configuration and compilation.** Configuration checks the version rules below. View
   templates check the required page-skin types and concepts, including tiles. A missing type fails with a member
   error; an invalid implementation fails with the named concept. Concept comments point to their component
   references. Do not maintain a second list of page-skin conformance checks. Main also checks the skin's screen
   against the board's screen.

6. **Main supplies the tiles' screens.** `TilesScreen` receives the enabled screen list from main. A skin knows how
   to draw each `ScreenId` but does not choose enabled screens. Optional IDs are guarded by the component switch.

7. **Retire a component in one commit.** Delete its folder, main's `REQUIRES`/integration guards/wiring, and every
   official skin's pages for it. Record the removal in SKIN_CHANGES as code outside authors may delete. An installed
   skin that still requires a deleted component receives ESP-IDF's component-resolution error naming the requiring
   skin. Removing only main's dependency while leaving the component folder is not a retirement.

8. **Maintain official skins in the firmware repository.** Piccolo is the default, always maintained so the firmware
   has a skin. Silk Taupe is an independent official component for the same screen. A fix in one is not
   automatically applied to the other. `check_code.sh` checks every official skin, and `check_skin_api.sh` requires
   their `AVELA_SKIN_API_VERSION` values to equal the numeric project version. TelchinDomus lists only officially
   supported skins.

9. **The firmware's licence is MIT.** Outside skins, including paid skins, may choose their own licence. Their
   assets retain their own licence requirements.

## Versions and skin compatibility

The firmware decides which skin APIs it accepts. A skin cannot predict compatibility with future firmware, so it
states the firmware API version it implements and lets the target firmware check it.

The top-level `CMakeLists.txt` sets `PROJECT_VER` to `X.Y.Z` for releases or `X.Y.Z-dev` during development, before
`project(avela_firmware)`. Compatibility checks use its numeric part without `-dev`; there is no separate skin API
counter. A skin's own repository or manifest version is independent. [Releases and versions](RELEASE.md) describes
version changes and release tags.

`AVELA_SKIN_API_VERSION` identifies the firmware version whose page-skin API it implements.
`supported_skin_api_versions` is the firmware's comma-separated numeric version constraint. Supported operators are
`>=`, `>`, `<=`, `<`, `==`, and `!=`; every clause must hold. For example, `>=1.4.0,!=1.5.0` excludes one API
version without listing every acceptable release. A skin API newer than the firmware always fails.

### When the lower bound changes

Advance the lower bound to the new firmware version in the same commit that requires a skin to add or change
something. This includes a required component/page skin, a setter signature, a value's range/units/meaning, redraw
semantics, or shared pieces such as `HeaderSkin`, `TilesSkin`, Surface, and `ScreenId`.

Removing a component/page skin does not advance the lower bound. A skin that references removed code fails at its
missing header or dependency and must delete that implementation. Firmware changes invisible to skins, including
radio fixes, settings format, partitions, or a major version change, do not advance the lower bound either.

### Migration instructions

Every incompatible change includes instructions in the same commit. The owning component updates its
`docs/developer/SKIN.md` with page purpose, setters, values/ranges/units, redraw behavior, control hints, and
reference implementation. A component is not complete without that contract; retiring it removes its documentation
too.

[Skin changes](SKIN_CHANGES.md) contains one entry per version that advances the lower bound, linking to the
relevant component contracts. Removals are listed as “you may delete.” Preserve version headings because the check
reads them.

### Enforcement

At configuration, CMake compares the chosen skin version with the firmware and constraint. Failure names the skin,
its implemented API, the firmware version, and the supported constraint, or tells the user to update a firmware
older than the requested API. The migration error points to SKIN_CHANGES. No C++ version parser is needed.

`check_skin_api.sh` requires a contract file for each component with a skin concept and for `avela_ui`. If a
skin-facing header or shared UI header changed since the latest v* release tag, the project version must be newer
and the changelog must contain that version. Unless the entry says `Nothing to change`, the supported lower bound
must equal the new version. Official skins must state the numeric project version without `-dev`.

## Every case

| Situation | Result |
|---|---|
| Component enabled; skin supplies its pages | Build and concept checks run |
| Component enabled; required page skin missing | Compile error |
| Component disabled; skin contains its pages | Those page sources/aliases are excluded |
| Component retired; skin still requires it | Configuration error naming the requiring component |
| API outside supported constraint | Configuration error pointing to migration instructions |
| API newer than firmware | Configuration error requiring newer firmware |
| Skin targets a different screen size | Compile-time screen-size failure |

Outside skins can be local or managed dependencies. ESP-IDF configures discovered components even when they are not
selected, so stale dependencies in an unselected installed skin still require attention.
[Writing a skin](SKINS.md#developing) gives a complete local-directory example.
