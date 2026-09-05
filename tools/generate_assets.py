#!/usr/bin/env python3
"""Render original vector tube numerals into native Pebble PNG assets.

Requires Pillow. No fonts, downloaded artwork, or original binary are needed.
Coordinates below follow the original 48 x 80 digit artwork recovered from the PBW.
Color PNG indices deliberately encode coverage; C supplies the actual palette.
"""
from pathlib import Path

from PIL import Image, ImageChops, ImageDraw, ImageFilter

ROOT = Path(__file__).resolve().parents[1]
SCALE = 6
PATHS = [
    [('M', 24, 4), ('C', 11, 4, 4, 13, 4, 26), ('L', 4, 54),
     ('C', 4, 69, 12, 76, 24, 76), ('C', 36, 76, 44, 69, 44, 54), ('L', 44, 26),
     ('C', 44, 13, 37, 4, 24, 4)],
    [('M', 4, 16), ('L', 20, 4), ('L', 24, 4), ('L', 24, 78)],
    [('M', 2, 4), ('L', 24, 4), ('C', 37, 4, 44, 12, 44, 24),
     ('C', 44, 37, 35, 40, 24, 40), ('C', 11, 40, 4, 51, 4, 65), ('L', 4, 76), ('L', 46, 76)],
    [('M', 2, 4), ('L', 24, 4), ('C', 50, 4, 50, 40, 24, 40), ('L', 10, 40),
     ('M', 24, 40), ('C', 50, 40, 50, 76, 24, 76), ('L', 2, 76)],
    [('M', 4, 2), ('L', 4, 24), ('C', 4, 37, 14, 40, 28, 40), ('L', 44, 40),
     ('M', 44, 2), ('L', 44, 78)],
    [('M', 46, 4), ('L', 4, 4), ('L', 4, 40), ('L', 24, 40),
     ('C', 50, 40, 50, 76, 24, 76), ('L', 2, 76)],
    [('M', 38, 4), ('C', 17, -1, 4, 16, 4, 39), ('L', 4, 56),
     ('C', 4, 83, 44, 83, 44, 56), ('C', 44, 32, 4, 32, 4, 56)],
    [('M', 2, 4), ('L', 24, 4), ('C', 37, 4, 44, 14, 44, 26), ('L', 44, 78)],
    [('M', 24, 40), ('C', -3, 40, -3, 4, 24, 4), ('C', 51, 4, 51, 40, 24, 40),
     ('C', -3, 40, -3, 76, 24, 76), ('C', 51, 76, 51, 40, 24, 40)],
    [('M', 10, 76), ('C', 31, 81, 44, 64, 44, 41), ('L', 44, 24),
     ('C', 44, -3, 4, -3, 4, 24), ('C', 4, 48, 44, 48, 44, 24)]
]


def numeral(digit, size):
    w, h = size
    image = Image.new('L', (w * SCALE, h * SCALE))
    draw = ImageDraw.Draw(image)
    inset_x = inset_y = round(w * .065)
    def point(x, y):
        return ((inset_x + x / 48 * (w - 1 - 2 * inset_x)) * SCALE,
                (inset_y + y / 80 * (h - 1 - 2 * inset_y)) * SCALE)
    stroke = round(w * .17 * SCALE)
    paths, points = [], []
    current = (0, 0)
    for command in PATHS[digit]:
        if command[0] == 'M':
            if points:
                paths.append(points)
            current = command[1:]
            points = [point(*current)]
        elif command[0] == 'L':
            current = command[1:]
            points.append(point(*current))
        else:
            x0, y0 = current
            x1, y1, x2, y2, x3, y3 = command[1:]
            for n in range(1, 61):
                t = n / 60
                u = 1 - t
                points.append(point(u**3*x0 + 3*u*u*t*x1 + 3*u*t*t*x2 + t**3*x3,
                                    u**3*y0 + 3*u*u*t*y1 + 3*u*t*t*y2 + t**3*y3))
            current = (x3, y3)
    paths.append(points)
    for points in paths:
        draw.line(points, fill=255, width=stroke, joint='curve')
        # Pillow's wide polyline joins can leave hairline gaps between short
        # Bezier samples. Fill every interior join before reducing resolution.
        r = stroke / 2
        for x, y in points[1:-1]:
            draw.ellipse((x-r, y-r, x+r, y+r), fill=255)
    return image.resize(size, Image.Resampling.LANCZOS)


def tube(size):
    # Original construction: rectangular envelope, overlapping large rings,
    # two inner rings, and the same horizontal / vertical electrode lines.
    w, h = size
    inset = round(w * .065)
    image = Image.new('L', size)
    draw = ImageDraw.Draw(image)
    wire_width = 3 if w >= 90 else 2
    def p(x, y):
        return (round(inset + x/48 * (w-1-2*inset)),
                round(inset + y/80 * (h-1-2*inset)))
    draw.rectangle((*p(0, 0), *p(48, 80)), outline=255, width=wire_width)
    for bounds in [(0, 0, 48, 48), (0, 32, 48, 80), (16, 16, 32, 32), (16, 48, 32, 64)]:
        draw.ellipse((*p(*bounds[:2]), *p(*bounds[2:])), outline=255, width=wire_width)
    for x in [16, 32]:
        draw.line((p(x, 0), p(x, 80)), fill=255, width=wire_width)
    for y in [16, 32, 48, 64]:
        draw.line((p(0, y), p(48, y)), fill=255, width=wire_width)
    return image


def palette():
    bg, glow, core = (0, 0, 85), (255, 255, 0), (255, 255, 85)
    def mix(a, b, amount):
        return tuple(round((x*(255-amount)+y*amount)/255/85)*85 for x, y in zip(a, b))
    colors = [bg]
    colors += [mix(bg, glow, a) for a in (42, 72, 106, 143, 181, 219, 255)]
    colors += [mix(glow, core, a) for a in (32, 64, 96, 128, 160, 192, 224, 255)]
    return [channel for color in colors for channel in color]


def generate():
    for name, size, color in [('small', (67, 80), True), ('emery', (93, 108), True),
                              ('bw', (67, 80), False)]:
        directory = ROOT / 'resources/images' / name
        directory.mkdir(parents=True, exist_ok=True)
        frame = tube(size)
        for digit in range(10):
            body = numeral(digit, size)
            # The active glyph is an outline, with two bright edges and a hollow
            # center along every stroke. Keep its edges heavier than the fine grid.
            # Wirework is 2/3px; the lit numeral edge is 3/4px (small/Emery).
            interior = body.filter(ImageFilter.MinFilter(9 if size[0] >= 90 else 7))
            mask = ImageChops.subtract(body, interior)
            if color:
                bloom = mask.filter(ImageFilter.GaussianBlur(size[0]*.033))
                ambient = mask.filter(ImageFilter.GaussianBlur(size[0]*.12))
                pixels = []
                for ink, halo, haze, border, solid, hollow in zip(mask.getdata(), bloom.getdata(), ambient.getdata(), frame.getdata(), body.getdata(), interior.getdata()):
                    if ink >= 250:
                        index = 15
                    elif ink >= 32:
                        index = 8 + min(6, ink*7//255)
                    else:
                        index = min(7, max((halo*12)//255, (haze*9)//255))
                    if border and solid < 32:
                        index = max(index, 2)
                    if hollow >= 128:
                        index = 0
                    pixels.append(index)
                result = Image.new('P', size)
                result.putpalette(palette())
                result.putdata(pixels)
                result.save(directory / f'{digit}.png', bits=4, optimize=False)
            else:
                # A narrow clear gap stops fine wires merging into the active number.
                clearance = body.filter(ImageFilter.MaxFilter(3))
                result = Image.new('1', size)
                result.putdata([255 if ink >= 128 or (border and nearby < 64) else 0
                                for ink, nearby, border in zip(mask.getdata(), clearance.getdata(), frame.getdata())])
                result.save(directory / f'{digit}.png')

    # Native 25px four-tube icon, following the watchface's own art direction.
    for kind in ('color', 'bw'):
        icon = Image.new('RGB', (25, 25), 'black')
        draw = ImageDraw.Draw(icon)
        rim = '#555500' if kind == 'color' else 'white'
        ink = '#FFFFAA' if kind == 'color' else 'white'
        for x in (1, 14):
            for y in (0, 13):
                draw.rounded_rectangle((x, y, x+9, y+11), radius=2, outline=rim)
        draw.line([(5, 3), (6, 2), (6, 9)], fill=ink, width=2)
        draw.line([(17, 3), (18, 2), (20, 2), (21, 4), (17, 8), (17, 9), (21, 9)], fill=ink)
        draw.line([(4, 15), (7, 15), (8, 17), (6, 18), (8, 19), (7, 22), (4, 22)], fill=ink)
        draw.line([(18, 15), (17, 19), (21, 19)], fill=ink)
        draw.line([(20, 16), (20, 22)], fill=ink)
        icon.save(ROOT / f'resources/images/menu_icon~{kind}.png')


if __name__ == '__main__':
    generate()
