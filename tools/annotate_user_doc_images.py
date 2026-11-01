"""Add vector callouts to the CAD front view and native firmware screen.

Run after refreshing the tracked screen and front-panel inputs. Raster images
stay unchanged; the SVG diagrams embed them beneath vector labels and leaders.
"""

import base64
from html import escape
import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
PANEL_MEDIA = ROOT / "components/avela_boards/docs/user/media"
RADIO_MEDIA = ROOT / "components/avela_radio/docs/user/media"


def start(width, height, title, description):
    return (
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" '
        f'height="{height}" viewBox="0 0 {width} {height}" role="img" '
        'aria-labelledby="title description">\n'
        f'<title id="title">{escape(title)}</title>'
        f'<desc id="description">{escape(description)}</desc>\n'
        '<style>text{font-family:system-ui,sans-serif;fill:#243a3c}'
        '.label{font-size:17px}.number{font-size:13px;font-weight:600;'
        'fill:#f8f6ef;text-anchor:middle;dominant-baseline:central}'
        '.leader{fill:none;stroke:#8c6037;stroke-width:1.4;'
        'stroke-linecap:round;stroke-linejoin:round}'
        '.region{fill:none;stroke:#d2af6d;stroke-width:1}</style>\n'
    )


def embed(path, x, y, width, height, pixelated=False):
    data = base64.b64encode(path.read_bytes()).decode()
    style = ' style="image-rendering:pixelated"' if pixelated else ""
    return (
        f'<image x="{x}" y="{y}" width="{width}" height="{height}" '
        f'href="data:image/png;base64,{data}"{style}/>'
    )


def label(number, text, x, y):
    return (
        f'<circle cx="{x}" cy="{y}" r="11" fill="#0f4248"/>'
        f'<text class="number" x="{x}" y="{y}">{number}</text>'
        f'<text class="label" x="{x + 18}" y="{y + 6}">{escape(text)}</text>'
    )


def front_panel():
    data = json.loads((PANEL_MEDIA / "front-panel-anchors.json").read_text())
    if (
        hashlib.sha256((PANEL_MEDIA / "front-panel.png").read_bytes()).hexdigest()
        != data["image_sha256"]
    ):
        raise ValueError(
            "The front-panel render and projected anchors must be refreshed together."
        )
    svg = start(
        900,
        470,
        "Avela Piccolo front panel",
        "1 knob; 2 Back button; 3 display; 4 speaker; 5 reset pinhole.",
    )
    svg += embed(PANEL_MEDIA / "front-panel.png", 10, 65, 880, 880 * 600 / 1400)
    for key, number, text, x, y, top in [
        ("knob", 1, "Knob", 70, 32, True),
        ("back", 2, "Back", 300, 450, False),
        ("screen", 3, "Display", 470, 32, True),
        ("speaker", 4, "Speaker", 740, 32, True),
        ("reset", 5, "Reset pinhole", 280, 32, True),
    ]:
        px, py = data["anchors"][key]
        px, py = 10 + px * 880 / 1400, 65 + py * 880 / 1400
        sy = y + 15 if top else y - 15
        bend = py - 35 if top else py + 35
        svg += (
            f'<path class="leader" d="M{x},{sy} V{bend:.2f} H{px:.2f} V{py:.2f}"/>'
            f'<circle cx="{px:.2f}" cy="{py:.2f}" r="3" fill="#d2af6d"/>'
        )
        svg += label(number, text, x, y)
    (PANEL_MEDIA / "front-panel-guide.svg").write_text(svg + "</svg>\n")


def screen():
    svg = start(
        790,
        370,
        "Radio screen explained",
        "A native 320 by 170 screen with callouts: 1 LIVE; 2 volume; 3 Wi-Fi; "
        "4 time and date; 5 station and track; 6 stream format; 7 spectrum.",
    )
    svg += embed(RADIO_MEDIA / "radio.png", 235, 95, 320, 170, pixelated=True)
    for number, text, x, width, label_x in [
        (1, "LIVE", 124, 28, 270),
        (2, "Volume", 155, 27, 390),
        (3, "Wi-Fi", 184, 19, 500),
        (4, "Time & date", 205, 107, 650),
    ]:
        target_x = 235 + x + width / 2
        svg += (
            f'<rect class="region" x="{235 + x}" y="96" '
            f'width="{width}" height="22" rx="2"/>'
        )
        svg += f'<path class="leader" d="M{label_x},40 L{target_x},77 V96"/>'
        svg += label(number, text, label_x, 25)
    svg += '<rect class="region" x="311" y="143" width="236" height="54" rx="2"/>'
    svg += '<path class="leader" d="M598,170 H547"/>' + label(
        5, "Station & track", 610, 170
    )
    svg += '<rect class="region" x="311" y="126" width="120" height="17" rx="2"/>'
    svg += '<path class="leader" d="M200,135 H216 V123 H311 V126"/>' + label(
        6, "Stream format", 30, 135
    )
    svg += '<rect class="region" x="243" y="200" width="304" height="63" rx="2"/>'
    svg += '<path class="leader" d="M335,310 V282 H395 V263"/>' + label(
        7, "Spectrum", 335, 325
    )
    (RADIO_MEDIA / "screen-guide.svg").write_text(svg + "</svg>\n")


if __name__ == "__main__":
    front_panel()
    screen()
