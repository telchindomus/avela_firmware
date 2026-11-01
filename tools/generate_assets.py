"""Generate the official skins' binary assets from the sources in each skin's assets/.

Per skin component and screen (SKINS; the firmware picks the skin in menuconfig: Avela skin, the screen comes with
the device), from components/<skin>/assets/:
- Manrope (variable TTF) -> one anti-aliased VLW font per role (title, body, caption, ...), in the format
  LovyanGFX's VLWfont reads (Processing's .vlw: big-endian header, glyph table, 8-bit alpha bitmaps);
- the Avela symbol (logo/avela_symbol.svg: sun, wave, bars, no background square) -> rendered at its
  display size and stored as raw ARGB8888.

Needs Pillow and resvg-py, e.g. `pip install pillow resvg-py`; then:  python3 tools/generate_assets.py
Outputs go to components/<skin>/assets/generated/<screen>/ and are committed, so a firmware build never needs Python.
"""

import io
import re
import struct
from dataclasses import dataclass
from pathlib import Path

import resvg_py
from PIL import Image, ImageDraw, ImageFont

PROJECT_DIRECTORY = Path(__file__).resolve().parent.parent
# inside each skin's assets/ directory
FONT_FILE = Path("fonts") / "Manrope-wght.ttf"
SYMBOL_FILE = Path("logo") / "avela_symbol.svg"
OUTPUT_DIRECTORY = Path("generated")

# Printable ASCII plus the few typographic characters the UI uses.
CHARACTERS = [chr(code) for code in range(0x20, 0x7F)] + ["·", "°", "–", "…", "‹", "›"]


# Big numbers only (the weather's temperature): digits, minus and the degree sign keep the large font small in flash.
NUMBER_CHARACTERS = [chr(code) for code in range(ord("0"), ord("9") + 1)] + [" ", "-", "°"]


@dataclass(frozen=True)
class FontSpec:
    weight: int
    pixel_size: int
    characters: tuple = tuple(CHARACTERS)


# The font roles the skins use. Keep in sync with Fonts (<skin>/fonts.hpp), assets.hpp and
# components/<skin>/CMakeLists.txt; every skin defines all of them for each of its screens.
FONT_ROLES = ["display", "headline", "title", "heading", "figure", "subtitle", "list_item", "body", "body_strong",
              "caption", "badge"]


@dataclass(frozen=True)
class SymbolSpec:
    height: int                  # output height in pixels; the width follows the symbol's proportions
    measure_size: int = 2048     # render size used to find the symbol's bounds inside its viewBox
    # Small-size boldening of the wave (an outline in its own colour, in viewBox units): at 20 px the wave is
    # only ~1 px thick and part of it faded out on the display. The bars are thick enough as they are.
    wave_outline_width: float = 0.0


@dataclass(frozen=True)
class ScreenAssets:
    fonts: dict  # role -> FontSpec
    symbol: SymbolSpec


# The official skins' typography and logo size on the 320x170 screen: the same for both (each has its own font and
# logo sources in its assets/).
LANDSCAPE_320X170 = ScreenAssets(
    fonts={
        "display": FontSpec(700, 40, tuple(NUMBER_CHARACTERS)),  # the weather's temperature
        "headline": FontSpec(700, 22),     # screen headline ("No Wi-Fi yet")
        "title": FontSpec(700, 19),        # station name on the player
        "heading": FontSpec(700, 15),      # brand name, selected list row, station monogram
        "figure": FontSpec(600, 13),       # clock, ring values
        "subtitle": FontSpec(600, 12),     # stream title (Medium 12 looked blurry, user)
        "list_item": FontSpec(500, 12),    # unselected list rows
        "body": FontSpec(500, 11),         # date, labels
        "body_strong": FontSpec(600, 11),  # values next to labels
        "caption": FontSpec(500, 10),      # format, genre, hints
        "badge": FontSpec(800, 9),         # LIVE / INFO / SETUP / UPDATE pills
    },
    symbol=SymbolSpec(height=20, wave_outline_width=14.0),
)

# skin component -> screen -> what its assets for that screen look like
SKINS = {
    "avela_skin_piccolo": {"landscape_320x170": LANDSCAPE_320X170},
    "avela_skin_piccolo_silk_taupe": {"landscape_320x170": LANDSCAPE_320X170},
}


def vlw_font(font_path, spec):
    font = ImageFont.truetype(str(font_path), spec.pixel_size)
    font.set_variation_by_axes([spec.weight])
    ascent, descent = font.getmetrics()
    records = []
    bitmaps = []
    # LovyanGFX finds a glyph by binary search, so the glyphs go in code point order
    characters = sorted(set(spec.characters), key=ord)
    for character in characters:
        left, top, right, bottom = font.getbbox(character, anchor="ls")
        width, height = max(0, right - left), max(0, bottom - top)
        advance = round(font.getlength(character))
        glyph = Image.new("L", (max(1, width), max(1, height)), 0)
        if width and height:
            ImageDraw.Draw(glyph).text((-left, -top), character, font=font, fill=255, anchor="ls")
            bitmap = glyph.tobytes()
        else:
            bitmap = b""
        # glyph record: unicode, height, width, xAdvance, dY (top above baseline), dX (left offset), padding
        records.append(struct.pack(">7i", ord(character), height if bitmap else 0, width if bitmap else 0,
                                   advance, -top, left, 0))
        bitmaps.append(bitmap)
    # header: glyph count, encoder version, font size, unused, ascent, descent
    header = struct.pack(">6i", len(characters), 11, spec.pixel_size, 0, ascent, descent)
    return header + b"".join(records) + b"".join(bitmaps)


def render_svg(svg, width, height):
    return Image.open(io.BytesIO(bytes(resvg_py.svg_to_bytes(svg_string=svg, width=width, height=height)))).convert("RGBA")


def symbol_pixels(symbol_path, spec):
    """The symbol rendered straight at its display size, its viewBox cropped to the drawn shapes."""
    svg = symbol_path.read_text()
    if spec.wave_outline_width:
        wave_color = re.search(r'<path id="wave" fill="([^"]+)"', svg).group(1)
        svg = svg.replace('<path id="wave" ',
                          f'<path id="wave" stroke="{wave_color}" stroke-width="{spec.wave_outline_width:g}" '
                          'stroke-linejoin="round" ', 1)
    view_box = [float(value) for value in re.search(r'viewBox="([^"]+)"', svg).group(1).split()]
    measured = render_svg(svg, spec.measure_size, spec.measure_size)
    left, top, right, bottom = measured.getchannel("A").getbbox()
    units_per_pixel = view_box[2] / spec.measure_size
    crop = (view_box[0] + left * units_per_pixel, view_box[1] + top * units_per_pixel,
            (right - left) * units_per_pixel, (bottom - top) * units_per_pixel)
    width = round(crop[2] * spec.height / crop[3])
    cropped_svg = re.sub(r'viewBox="[^"]+"', 'viewBox="%.3f %.3f %.3f %.3f"' % crop, svg, count=1)
    cropped_svg = re.sub(r'width="[^"]+" height="[^"]+"', f'width="{width}" height="{spec.height}"', cropped_svg, count=1)
    symbol = render_svg(cropped_svg, width, spec.height)
    # resvg gives straight (non-premultiplied) RGBA; LovyanGFX argb8888_t is {b, g, r, a} in memory
    red, green, blue, alpha_channel = symbol.split()
    return symbol.size, Image.merge("RGBA", (blue, green, red, alpha_channel)).tobytes()


def generate_screen_assets(skin_assets_directory, screen, assets):
    name = f"{skin_assets_directory.parent.name}/{screen}"
    assert sorted(assets.fonts) == sorted(FONT_ROLES), f"{name}: fonts must cover exactly {FONT_ROLES}"
    output_directory = skin_assets_directory / OUTPUT_DIRECTORY / screen
    output_directory.mkdir(parents=True, exist_ok=True)
    for role in FONT_ROLES:
        data = vlw_font(skin_assets_directory / FONT_FILE, assets.fonts[role])
        (output_directory / f"font_{role}.vlw").write_bytes(data)
        print(f"{name}/font_{role}.vlw: {len(data)} bytes")
    (width, height), pixels = symbol_pixels(skin_assets_directory / SYMBOL_FILE, assets.symbol)
    (output_directory / "avela_logo.argb").write_bytes(pixels)
    (output_directory / "avela_logo_size.hpp").write_text(
        "// Generated by tools/generate_assets.py -- do not edit.\n#pragma once\n\n"
        f"namespace avela::assets {{\ninline constexpr int logo_width = {width};\n"
        f"inline constexpr int logo_height = {height};\n}}  // namespace avela::assets\n")
    print(f"{name}/avela_logo.argb: {width}x{height}, {len(pixels)} bytes")


def main():
    for skin, screens in SKINS.items():
        skin_assets_directory = PROJECT_DIRECTORY / "components" / skin / "assets"
        for source in (FONT_FILE, SYMBOL_FILE):
            assert (skin_assets_directory / source).exists(), f"missing {skin_assets_directory / source}"
        for screen, assets in screens.items():
            generate_screen_assets(skin_assets_directory, screen, assets)


if __name__ == "__main__":
    main()
