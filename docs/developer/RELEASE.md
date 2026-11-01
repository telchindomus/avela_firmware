# Avela firmware: releases and versions

Release work uses `devel` for development, `feature/<name>` for features, and `main` for releases tagged `vX.Y.Z`.
Run the release and build commands from the repository root with an activated ESP-IDF environment.
[Build and flash](BUILD.md) covers local builds, host prerequisites, and flashing.

## Firmware and skin versions

The top-level `CMakeLists.txt` sets `PROJECT_VER` before `project(avela_firmware)`. A release uses `X.Y.Z`;
development uses `X.Y.Z-dev`, where the number identifies the next release. The Info screen, firmware update page,
and stream User-Agent report the full value, for example `Avela/1.0.1-dev`.

The skin API uses the numeric part without `-dev`. Official skins set `AVELA_SKIN_API_VERSION` to that number.
`tools/check_skin_api.sh` checks the version's form and official skin versions. The acceptance constraint,
lower-bound changes, and migration instructions belong to
[Skin architecture](SKIN_ARCHITECTURE.md#versions-and-skin-compatibility).

Write incompatible changes on `devel` under its numeric version in [Skin changes](SKIN_CHANGES.md), with the
matching `supported_skin_api_versions` lower bound. When the release number changes, the release script moves that
version heading and lower bound to the selected release number.

## Make a release

Start on `devel` with committed changes and the required [host tools](BUILD.md#host-prerequisites). The release
command creates commits and a tag, merges branches, and pushes `main`, `devel`, and the tag to `origin`:

```sh
cicd/release.sh patch
```

Use `cicd/release.sh X.Y.Z` for an explicit version, or replace `patch` with `minor` or `major`. With `1.0.1-dev`,
`patch` releases `1.0.1`, `minor` selects `1.1.0`, and `major` selects `2.0.0`.

Before creating the release branch, the script checks the working tree is clean, `devel` is checked out, ESP-IDF is
active, and a Git email is configured. It fetches `origin`, checks that the local `devel` is the same commit as the
remote one and that the tag is absent, and rejects a version older than the current development number.

1. Create `release/<version>`. Set the firmware and all official skin versions to `X.Y.Z`; move the development
   version's skin-change heading and lower bound if the number changed.
2. Commit the release version on the release branch. Run `cicd/test_unit.sh --source <repository> --tag
   release/<version>`, which builds and runs the code gate in a fresh clone of that commit, never in the working
   copy. On failure, return to `devel`, delete the release branch, and stop before the tag.
3. Create the annotated `vX.Y.Z` tag. Pull `main` from `origin` and merge the release branch into it.
4. Merge the release branch into `devel`. Commit `X.Y.(Z+1)-dev` as the next firmware version, with the matching
   numeric official skin versions.
5. Delete the release branch and push `main`, `devel`, and the tag to `origin`.

## Build release images

The build script clones the selected remote ref into `<CLONES_DIR>/build-<time>` and builds every official skin with
`sdkconfig.defaults` plus that skin's selection. The clone is removed afterward:

```sh
cicd/build.sh --version X.Y.Z
```

`--tag vX.Y.Z` selects the same tag; `--tag` can also name a branch. With neither option, the script uses
`BUILD_BRANCH`, which defaults to `main`. Local uncommitted changes are not included.

Output goes to `<ARTIFACTS_DIR>/<version>/`, where `ARTIFACTS_DIR` defaults to `<BUILD_DIR>/artifacts`. Each skin
has its own folder:

| File | Use |
|---|---|
| `avela_firmware.bin` | Network update, or USB app image together with initial OTA metadata |
| `ota_data_initial.bin` | OTA metadata for a USB update that keeps settings |
| `bootloader/bootloader.bin` and `partition_table/partition-table.bin` | First installation by parts |
| `flash_args` | Offsets and settings for flashing the separate images from this folder |
| `avela_firmware_full.bin` | Merged image for a first installation; erases saved settings |

To flash the separate images, run in the selected skin's output folder:

```sh
esptool.py --chip esp32 --port <PORT> write_flash @flash_args
```

[Build and flash](BUILD.md#usb-update-that-keeps-settings) defines the USB offsets and settings-preservation rules.
Follow its [first-installation procedure](BUILD.md#full-image-for-first-installation) for the merged image.

Beside the skin folders, `checksums.sha256` covers the generated image and flash-argument files; `version.txt`
records the version. The `latest` link under `ARTIFACTS_DIR` points to the most recently completed build. Building
an existing version replaces that version's output; an interactive run asks first.

## Test a branch or tag

`cicd/test_unit.sh` clones `devel` from the configured repository into `<CLONES_DIR>/test-<time>`, builds it, and
runs `tools/check_code.sh`. Use `--tag <tag-or-branch>` to select another ref, and `--source <path-or-URL>` to clone
from another repository. It never tests the working copy. The test uses a full clone so the skin check can compare
with release tags. The clone is removed after a pass and kept after a failure.

## Build and test configuration

The build and test scripts read `cicd/.env` when present. Copy `cicd/.env.example` to configure local values;
`cicd/.env` is ignored by Git. The release script uses the working repository and its `origin` remote directly; its
gates clone that repository.

| Variable | Default | Used by |
|---|---|---|
| `REPO_URL` | The working repository's `origin` URL | Build and test |
| `BUILD_DIR` | `cicd/build` in the repository | Build and test |
| `ARTIFACTS_DIR` | `<BUILD_DIR>/artifacts` | Build |
| `CLONES_DIR` | `<BUILD_DIR>/clones` | Build and test |
| `BUILD_BRANCH` | `main` | Build without an explicit ref |
