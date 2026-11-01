#!/usr/bin/env python3
"""Render Piccolo at native resolution and collect the guide illustrations.

The existing external skin toolbox compiles the current component sources. This
script copies its PNGs without resizing and records source and output hashes.
"""

import argparse
import hashlib
import json
from pathlib import Path
import shutil
import struct
import subprocess


ROOT = Path(__file__).resolve().parent.parent
SCENES = {
    "player_playing": "components/avela_radio/docs/user/media/radio.png",
    "stations": "components/avela_radio/docs/user/media/stations.png",
    "info_connected": "components/avela_system_info/docs/user/media/info.png",
    "wifi_setup_up": "components/avela_network/docs/user/media/wifi-setup.png",
    "firmware_update_waiting": (
        "components/avela_firmware_update/docs/user/media/update.png"
    ),
    "firmware_update_receiving": (
        "components/avela_firmware_update/docs/user/media/update-receiving.png"
    ),
    "tiles": "docs/user/media/screen-menu.png",
    "weather_day": "components/avela_weather/docs/user/media/weather.png",
}
SKINS = (("avela_skin_piccolo", "piccolo", ""),)


def git(repository, *arguments):
    return subprocess.check_output(
        ["git", "-C", str(repository), *arguments], text=True
    ).strip()


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def source_hashes(repository, component_sources=False):
    result = {}
    for name in git(repository, "ls-files").splitlines():
        path = repository / name
        if component_sources and not name.startswith("components/"):
            continue
        if path.suffix in {
            ".cpp",
            ".hpp",
            ".h",
            ".c",
            ".cmake",
            ".py",
            ".sh",
            ".vlw",
            ".argb",
        } or path.name in {
            "CMakeLists.txt",
            "Kconfig",
            "idf_component.yml",
            "requirements.txt",
        }:
            result[name] = digest(path)
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--toolbox",
        type=Path,
        required=True,
        help="existing avela_skin_toolbox checkout",
    )
    parser.add_argument(
        "--lovyangfx", type=Path, required=True, help="existing LovyanGFX checkout"
    )
    arguments = parser.parse_args()
    toolbox = arguments.toolbox.resolve()
    lovyangfx = arguments.lovyangfx.resolve()
    build = ROOT / "build/user-doc-images"
    build.mkdir(parents=True, exist_ok=True)
    manifest = {
        "firmware_commit": git(ROOT, "rev-parse", "HEAD"),
        "toolbox_commit": git(toolbox, "rev-parse", "HEAD"),
        "kind": (
            "host renders of official skin sources with toolbox sample data; "
            "no hardware capture"
        ),
        "width": 320,
        "height": 170,
        "scale": 1,
        "firmware_inputs_sha256": source_hashes(ROOT, component_sources=True),
        "toolbox_inputs_sha256": source_hashes(toolbox),
        "images": {},
    }
    for component, folder, suffix in SKINS:
        output = build / folder
        command = [
            str(toolbox / "render.sh"),
            "--skin",
            str(ROOT / "components" / component),
            "--components",
            str(ROOT / "components"),
            "--lovyangfx",
            str(lovyangfx),
            "--out",
            str(output),
            "--scale",
            "1",
        ]
        with (build / f"{folder}.log").open("w") as log:
            subprocess.run(
                command, cwd=ROOT, stdout=log, stderr=subprocess.STDOUT, check=True
            )
        for scene, name in SCENES.items():
            source = output / f"{scene}.png"
            data = source.read_bytes()
            if data[:8] != b"\x89PNG\r\n\x1a\n" or struct.unpack(
                ">II", data[16:24]
            ) != (320, 170):
                raise ValueError(f"Expected a native 320 by 170 PNG: {source}")
            destination = (ROOT / name).with_stem(Path(name).stem + suffix)
            destination.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(source, destination)
            manifest["images"][destination.relative_to(ROOT).as_posix()] = {
                "skin": component,
                "scene": scene,
                "sha256": digest(destination),
            }
    metadata = ROOT / "docs/user/media/screens.json"
    metadata.write_text(json.dumps(manifest, indent=2) + "\n")
    print(f"Rendered and collected {len(manifest['images'])} native screen images.")


if __name__ == "__main__":
    main()
