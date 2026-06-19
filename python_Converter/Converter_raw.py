# -*- coding: utf-8 -*-
"""
Por Alejandro Rebolledo
arebolledo@udd.cl
"""

from pathlib import Path
from PIL import Image

SPRITE_W = 120
SPRITE_H = 130

INPUT_DIR = Path("png")
OUTPUT_DIR = Path("data/cat")

TRANSPARENT_RGB = (255, 0, 255)

OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

def rgb888_to_rgb565(r: int, g: int, b: int) -> int:
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)

def convert_png_to_raw_rgb565(input_path: Path, output_path: Path) -> None:
    image = Image.open(input_path).convert("RGBA")
    image = image.resize((SPRITE_W, SPRITE_H), Image.Resampling.LANCZOS)

    with output_path.open("wb") as file:
        for y in range(SPRITE_H):
            for x in range(SPRITE_W):
                r, g, b, a = image.getpixel((x, y))

                if a < 128:
                    r, g, b = TRANSPARENT_RGB

                rgb565 = rgb888_to_rgb565(r, g, b)

                # Little-endian output:
                file.write(bytes([rgb565 & 0xFF, (rgb565 >> 8) & 0xFF]))

def main() -> None:
    png_files = sorted(INPUT_DIR.glob("cat*.png"))

    if not png_files:
        print("No PNG files found in input/")
        return

    for input_path in png_files:
        output_path = OUTPUT_DIR / f"{input_path.stem}.raw"
        convert_png_to_raw_rgb565(input_path, output_path)

        size_bytes = output_path.stat().st_size
        print(f"Converted {input_path} -> {output_path} ({size_bytes} bytes)")

if __name__ == "__main__":
    main()