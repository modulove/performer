#!/usr/bin/env python3
"""
Convert a TTF font to BitmapFont format for the PER|FORMER sequencer.
This generates a .h file compatible with the BitmapFont structure.
"""

import sys
from PIL import Image, ImageDraw, ImageFont
import argparse

def generate_bitmapfont(ttf_path, size, output_name, first_char=0x20, last_char=0x7E, all_caps=False, bold=False, y_offset_adjust=0):
    """
    Generate a BitmapFont .h file from a TTF font.

    Args:
        ttf_path: Path to the TTF file
        size: Font size in pixels
        output_name: Base name for the output (e.g., "miracode5pt7b")
        first_char: First character code (default: 0x20 = space)
        last_char: Last character code (default: 0x7E = ~)
        all_caps: Convert lowercase letters to uppercase (default: False)
        bold: Render bold by drawing twice with 1px offset (default: False)
        y_offset_adjust: Pixels to add to yOffset (positive = move down) (default: 0)
    """

    # Load the font
    try:
        font = ImageFont.truetype(ttf_path, size)
    except Exception as e:
        print(f"Error loading font: {e}")
        sys.exit(1)

    # Collect glyph data
    bitmap_data = []
    glyph_data = []
    current_offset = 0
    max_height = 0

    for char_code in range(first_char, last_char + 1):
        char = chr(char_code)

        # Convert lowercase to uppercase if all_caps is enabled
        render_char = char
        if all_caps and 0x61 <= char_code <= 0x7A:  # lowercase a-z
            render_char = char.upper()

        # Get the bounding box and metrics for this character
        bbox = font.getbbox(render_char)
        if bbox == (0, 0, 0, 0):
            # Empty glyph (like space)
            width, height = 1, 1
            x_offset, y_offset = 0, 0
            x_advance = font.getlength(render_char)
            if x_advance == 0:
                x_advance = size // 2  # Default advance for space
            pixels = [0]
        else:
            # Calculate dimensions
            x_min, y_min, x_max, y_max = bbox
            width = x_max - x_min
            height = y_max - y_min
            x_offset = x_min
            y_offset = -y_max + y_offset_adjust  # Y offset is negative from baseline, adjusted
            x_advance = font.getlength(render_char)

            # Render the glyph
            img = Image.new('L', (width, height), 0)
            draw = ImageDraw.Draw(img)
            draw.text((-x_min, -y_min), render_char, font=font, fill=255)

            # Render bold by drawing again with 1px offset
            if bold:
                draw.text((-x_min + 1, -y_min), render_char, font=font, fill=255)

            # Extract pixel data (1 bit per pixel, LSB-first packing)
            pixels = list(img.getdata())

        # Convert to 1-bit bitmap (LSB-first)
        bits = []
        for pixel in pixels:
            bits.append(1 if pixel > 127 else 0)

        # Pack bits into bytes (LSB-first)
        packed_bytes = []
        for i in range(0, len(bits), 8):
            byte_bits = bits[i:i+8]
            byte_val = 0
            for bit_pos, bit in enumerate(byte_bits):
                if bit:
                    byte_val |= (1 << bit_pos)
            packed_bytes.append(byte_val)

        # Store glyph info
        glyph_data.append({
            'offset': current_offset,
            'width': width,
            'height': height,
            'xAdvance': int(x_advance),
            'xOffset': int(x_offset),
            'yOffset': int(y_offset),
            'char': char,
            'code': char_code
        })

        bitmap_data.extend(packed_bytes)
        current_offset += len(packed_bytes)
        max_height = max(max_height, height)

    # Estimate yAdvance (line height)
    y_advance = max_height + 3  # Add some spacing

    # Generate the .h file
    header_guard = f"__{output_name.upper()}_H__"

    output = []
    output.append(f"#ifndef {header_guard}")
    output.append(f"#define {header_guard}")
    output.append("")
    output.append('#include "BitmapFont.h"')
    output.append("")

    # Bitmap array
    output.append(f"static uint8_t {output_name}_bitmap[] = {{")
    for i in range(0, len(bitmap_data), 12):
        row = bitmap_data[i:i+12]
        hex_str = ", ".join(f"0x{b:02X}" for b in row)
        if i + 12 < len(bitmap_data):
            output.append(f"  {hex_str},")
        else:
            output.append(f"  {hex_str}")
    output.append("};")
    output.append("")

    # Glyph array
    output.append(f"static BitmapFontGlyph {output_name}_glyphs[] = {{")
    for g in glyph_data:
        comment = f"// 0x{g['code']:02X} '{g['char']}'"
        output.append(f"  {{ {g['offset']:5}, {g['width']:3}, {g['height']:3}, {g['xAdvance']:3}, {g['xOffset']:4}, {g['yOffset']:4} }},   {comment}")
    output.append("};")
    output.append("")

    # Font struct
    output.append(f"static BitmapFont {output_name} = {{")
    output.append(f"  1, {output_name}_bitmap, {output_name}_glyphs, 0x{first_char:02X}, 0x{last_char:02X}, {y_advance}")
    output.append("};")
    output.append("")
    output.append(f"#endif // {header_guard}")
    output.append("")

    return "\n".join(output)

def main():
    parser = argparse.ArgumentParser(description='Convert TTF to BitmapFont format')
    parser.add_argument('ttf_file', help='Path to TTF font file')
    parser.add_argument('-s', '--size', type=int, default=8, help='Font size in pixels (default: 8)')
    parser.add_argument('-o', '--output', required=True, help='Output filename (without .h)')
    parser.add_argument('-n', '--name', help='Font variable name (defaults to output filename)')
    parser.add_argument('--first', type=lambda x: int(x, 0), default=0x20, help='First character code (default: 0x20)')
    parser.add_argument('--last', type=lambda x: int(x, 0), default=0x7E, help='Last character code (default: 0x7E)')
    parser.add_argument('--all-caps', action='store_true', help='Convert lowercase letters to uppercase')
    parser.add_argument('--bold', action='store_true', help='Render bold by drawing twice with 1px offset')
    parser.add_argument('--y-offset', type=int, default=0, help='Vertical offset adjustment in pixels (positive = move down)')

    args = parser.parse_args()

    font_name = args.name if args.name else args.output

    print(f"Converting {args.ttf_file} at size {args.size}px...")
    if args.all_caps:
        print("  - ALL CAPS mode enabled")
    if args.bold:
        print("  - Bold rendering enabled")
    if args.y_offset != 0:
        print(f"  - Vertical offset: {args.y_offset:+d} pixels")
    header_content = generate_bitmapfont(args.ttf_file, args.size, font_name, args.first, args.last, args.all_caps, args.bold, args.y_offset)

    output_file = f"{args.output}.h" if not args.output.endswith('.h') else args.output
    with open(output_file, 'w') as f:
        f.write(header_content)

    print(f"Generated {output_file}")

if __name__ == '__main__':
    main()
