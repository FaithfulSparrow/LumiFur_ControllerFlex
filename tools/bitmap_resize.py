#!/usr/bin/env python3
"""bitmap_resize.py — helper to resize XBM-style monochrome bitmap arrays.

Takes an existing PROGMEM bitmap array (as a C header snippet) and
produces a nearest-neighbor downscaled version for a target width/height.

Usage:
    python3 tools/bitmap_resize.py \
        --input src/assets/bitmaps.h \
        --symbol maw \
        --width 24 --height 6

Output is printed to stdout as a C array snippet.
"""

import argparse
import re
import sys
from typing import List, Tuple


def parse_bitmap(source: str, symbol: str) -> Tuple[int, int, List[int]]:
    """Find `symbol` in source text, return (width, height, byte_list)."""
    # Match: static uint8_t SYMBOL [] PROGMEM = { ... };
    # Heuristic: look for comments like // 'symbol', WxHpx before the array
    pattern = rf"(?:static\s+)?uint8_t\s+{re.escape(symbol)}\s*\[\]\s*(?:PROGMEM\s*)?=\s*\{{([^}}]+)\}}"
    match = re.search(pattern, source)
    if not match:
        raise ValueError(f"Symbol '{symbol}' not found.")

    body = match.group(1)
    # Parse comma-separated hex values
    bytes_list = []
    for token in body.split(","):
        token = token.strip()
        if token.startswith("0x") or token.startswith("0X"):
            bytes_list.append(int(token, 16))
        elif token.isdigit():
            bytes_list.append(int(token))

    # Try to extract width/height from a comment on the line above
    # e.g. // 'symbol', WxHpx
    size_pattern = rf"//\s*'[^']*'{re.escape(symbol)}[^']*'[^']*'?\s*,\s*(\d+)\s*x\s*(\d+)"
    size_match = re.search(size_pattern, source)
    if size_match:
        width = int(size_match.group(1))
        height = int(size_match.group(2))
    else:
        raise ValueError(f"Could not determine dimensions for '{symbol}'. "
                         f"Add a comment like // 'symbol', WxHpx before the array.")

    return width, height, bytes_list


def bytes_to_pixels(width: int, height: int, data: List[int]) -> List[List[bool]]:
    """Convert packed XBM bytes to a 2D bool grid."""
    byte_width = (width + 7) // 8
    pixels = []
    for y in range(height):
        row = []
        for x in range(width):
            byte_idx = y * byte_width + (x // 8)
            bit_mask = 0x80 >> (x % 8)
            row.append(bool(data[byte_idx] & bit_mask) if byte_idx < len(data) else False)
        pixels.append(row)
    return pixels


def pixels_to_bytes(pixels: List[List[bool]]) -> List[int]:
    """Convert a 2D bool grid back to packed XBM bytes."""
    height = len(pixels)
    width = len(pixels[0]) if height > 0 else 0
    byte_width = (width + 7) // 8
    data = [0] * (byte_width * height)
    for y in range(height):
        for x in range(width):
            if pixels[y][x]:
                byte_idx = y * byte_width + (x // 8)
                bit_mask = 0x80 >> (x % 8)
                data[byte_idx] |= bit_mask
    return data


def nearest_neighbor_scale(
    pixels: List[List[bool]], new_w: int, new_h: int
) -> List[List[bool]]:
    """Nearest-neighbor downscale."""
    old_h = len(pixels)
    old_w = len(pixels[0]) if old_h > 0 else 0
    result = [[False] * new_w for _ in range(new_h)]
    for y in range(new_h):
        src_y = int(y * old_h / new_h)
        for x in range(new_w):
            src_x = int(x * old_w / new_w)
            result[y][x] = pixels[src_y][src_x]
    return result


def format_bytes(data: List[int], bytes_per_line: int = 12) -> str:
    """Format bytes as a C array initializer."""
    lines = []
    for i in range(0, len(data), bytes_per_line):
        chunk = data[i : i + bytes_per_line]
        hex_str = ", ".join(f"0x{b:02x}" for b in chunk)
        lines.append(f"\t{hex_str}")
    return ",\n".join(lines)


def main():
    parser = argparse.ArgumentParser(description="Resize XBM-style PROGMEM bitmaps")
    parser.add_argument("--input", required=True, help="Path to C header file")
    parser.add_argument("--symbol", required=True, help="Bitmap variable name")
    parser.add_argument("--width", type=int, required=True, help="Target width")
    parser.add_argument("--height", type=int, required=True, help="Target height")
    args = parser.parse_args()

    with open(args.input) as f:
        source = f.read()

    old_w, old_h, data = parse_bitmap(source, args.symbol)
    print(f"// Original: {old_w}x{old_h} → Target: {args.width}x{args.height}",
          file=sys.stderr)

    pixels = bytes_to_pixels(old_w, old_h, data)
    scaled = nearest_neighbor_scale(pixels, args.width, args.height)
    new_data = pixels_to_bytes(scaled)
    byte_width = (args.width + 7) // 8

    print(f"// '{args.symbol}', {args.width}x{args.height}px (resized from {old_w}x{old_h})")
    print(f"static uint8_t {args.symbol} [] PROGMEM = {{")
    print(format_bytes(new_data))
    print("};")


if __name__ == "__main__":
    main()