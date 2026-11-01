# User-documentation illustrations

The user guide uses native screen renders and an annotated Piccolo front view. Screen images come from the actual
official skin sources, using the external avela_skin_toolbox host renderer and its sample data. They are examples,
not photographs or captures from a connected radio.

## Refresh the screens

Use the toolbox's host build environment and a LovyanGFX checkout matching the firmware's pinned version. Run from
the firmware root:

```sh
python3 tools/render_user_doc_screens.py --toolbox <toolbox-checkout> --lovyangfx <LovyanGFX-checkout>
```

The helper uses Piccolo at scale 1 and collects the eight screens used by the guide or its Radio diagram. It checks
that each PNG is 320 × 170, copies it unchanged into the owning component's user media folder, and records input
and output hashes in [screens.json](../user/media/screens.json). The screen-menu image belongs to the shared
guide. The toolbox's other scenes stay in the local build output rather than the documentation assets.

The shared instructions illustrate Piccolo. They also apply to Silk Taupe, whose colours and layout differ. Use
the toolbox directly for additional skins and states when checking their appearance; those outputs do not belong
in the maintained guide unless a page or diagram uses them.

The fixtures supply station, weather, network, clock and firmware-version values. They do not query a device or
contact the services represented on screen. Dynamic indicators can be captured at different animation steps;
identical source inputs do not guarantee an identical PNG hash.

## Rebuild the callouts

The [front-panel render](../../components/avela_boards/docs/user/media/front-panel.png) and its
[projected anchors](../../components/avela_boards/docs/user/media/front-panel-anchors.json) are the source inputs for
the control diagram. Replace those together when the illustrated enclosure changes. The anchor coordinates refer
to the image's native 1400 × 600 frame; they identify the knob, Back button, display, speaker and reset pinhole.

The Radio diagram embeds the freshly rendered Piccolo screen. Its boxes follow the header, tray and player layouts
in the skin. Check those positions if a layout changes, then rebuild both self-contained SVGs:

```sh
python3 tools/annotate_user_doc_images.py
```

This step needs only Python's standard library and the tracked image inputs. The SVGs embed their raster images,
so they remain usable when opened on their own. The Radio image inside its annotated diagram stays at its native
320 × 170 pixels.

## Review and verification

Inspect the images and the rendered guide after changing them. Keep native screen proportions, ensure labels and
leader endpoints remain readable, and distinguish sample values from details a user should read from their device.
The documentation check finds missing image paths; it cannot establish that a callout points to the right control.

Run the documentation and skin checks after updating the illustrations or their links:

```sh
tools/check_docs.sh
tools/check_skin_api.sh
```

The documentation images are committed with their authored pages. Intermediate host builds and meshes are local
build outputs. Regenerating illustrations is separate from exercising Wi-Fi, audio, browser uploads or rollback on
hardware.
