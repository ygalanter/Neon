# Neon App Store release

Author: **Comfortably Numb**

UUID: `13371337-a9cf-43ed-a84d-76f3dc91f626` (unchanged from the original Neon).

Version: **2.0.0**.

## Watchface bundle and icons

- [Neon.pbw](Neon.pbw): live-clock watchface bundle for Aplite, Basalt, Diorite, Flint, and Emery.
- [80 × 80 icon](icon_80x80.png) and [144 × 144 icon](icon_144x144.png): square PNGs made from the yellow-on-blue screenshot, preserving the full artwork and its proportions.

## Screenshots

All screenshots were captured from the emulators at **01:23**, at native resolution, with color correction disabled. Each GIF contains the exact static PNGs in their numbered order, with 1.5 seconds per frame and an infinite loop.

| Platform | Native size | Static screenshots | Animated slideshow |
| --- | --- | --- | --- |
| Aplite | 144 × 168 | [1](screenshots/aplite/01_white_on_black_0123.png), [2](screenshots/aplite/02_black_on_white_0123.png) | [GIF](screenshots/aplite/03_slideshow_0123.gif) |
| Basalt | 144 × 168 | [1](screenshots/basalt/01_gold_0123.png), [2](screenshots/basalt/02_cyan_0123.png), [3](screenshots/basalt/03_pink_0123.png), [4](screenshots/basalt/04_blue_on_white_0123.png) | [GIF](screenshots/basalt/05_slideshow_0123.gif) |
| Diorite | 144 × 168 | [1](screenshots/diorite/01_white_on_black_0123.png), [2](screenshots/diorite/02_black_on_white_0123.png) | [GIF](screenshots/diorite/03_slideshow_0123.gif) |
| Flint | 144 × 168 | [1](screenshots/flint/01_white_on_black_0123.png), [2](screenshots/flint/02_black_on_white_0123.png) | [GIF](screenshots/flint/03_slideshow_0123.gif) |
| Emery | 200 × 228 | [1](screenshots/emery/01_gold_0123.png), [2](screenshots/emery/02_cyan_0123.png), [3](screenshots/emery/03_pink_0123.png), [4](screenshots/emery/04_blue_on_white_0123.png) | [GIF](screenshots/emery/05_slideshow_0123.gif) |

## Color combinations

| Variant | Digit outline | Glow / wirework | Background |
| --- | --- | --- | --- |
| gold | `#FFFF55` | `#FFFF00` | `#000055` |
| cyan | `#AAFFFF` | `#00AAFF` | `#000000` |
| pink | `#FFAAFF` | `#FF00AA` | `#000000` |
| blue on white | `#000055` | `#0055AA` | `#FFFFFF` |

Monochrome screenshots show white on black, then black on white.

There are **14 screenshot PNGs and 5 GIFs**, plus the two square icons above. [manifest.json](manifest.json) records screenshot colors, file order, and SHA-256 checksums.

Only the screenshots use the fixed preview time; the delivered PBW uses the live clock.
