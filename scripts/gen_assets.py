#!/usr/bin/env python3
"""Generate resources/42.obj and resources/unicorn.bmp (no external deps)."""

from __future__ import annotations

import math
import struct
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RES = ROOT / "resources"


def write_bmp(path: Path, width: int, height: int, rgb_fn) -> None:
    row_stride = (width * 3 + 3) & ~3
    pixels = bytearray(row_stride * height)
    for y in range(height):
        for x in range(width):
            r, g, b = rgb_fn(x, y, width, height)
            i = y * row_stride + x * 3
            pixels[i : i + 3] = bytes((b, g, r))
    file_size = 54 + len(pixels)
    header = struct.pack(
        "<2sIHHIIiiHHIIiiII",
        b"BM",
        file_size,
        0,
        0,
        54,
        40,
        width,
        height,
        1,
        24,
        0,
        len(pixels),
        2835,
        2835,
        0,
        0,
    )
    path.write_bytes(header + pixels)


def hsv_to_rgb(h: float, s: float, val: float) -> tuple[int, int, int]:
    h = h % 1.0
    i = int(h * 6.0)
    f = h * 6.0 - i
    p = val * (1.0 - s)
    q = val * (1.0 - f * s)
    t = val * (1.0 - (1.0 - f) * s)
    i %= 6
    if i == 0:
        r, g, b = val, t, p
    elif i == 1:
        r, g, b = q, val, p
    elif i == 2:
        r, g, b = p, val, t
    elif i == 3:
        r, g, b = p, q, val
    elif i == 4:
        r, g, b = t, p, val
    else:
        r, g, b = val, p, q
    return (
        max(0, min(255, int(r * 255))),
        max(0, min(255, int(g * 255))),
        max(0, min(255, int(b * 255))),
    )


def unicorn_color(x: int, y: int, w: int, h: int) -> tuple[int, int, int]:
    """Glossy rainbow surface: flowing hue bands + baked specular sheen."""
    u = x / (w - 1)
    v = y / (h - 1)

    # Diagonal iridescent rainbow with a soft swirl
    hue = (u * 1.35 + v * 0.85 + 0.08 * math.sin(u * 12.0) + 0.06 * math.cos(v * 10.0)) % 1.0
    sat = 0.78 + 0.18 * math.sin((u + v) * math.pi * 2.0)
    value = 0.72 + 0.18 * math.sin((u * 3.0 - v * 2.0) * math.pi)

    # Glossy highlight streaks (anisotropic-looking sheen in the bitmap)
    sheen_a = math.exp(-((u * 0.7 + v * 1.3 - 0.85) ** 2) / 0.018)
    sheen_b = math.exp(-((u * 1.1 - v * 0.4 - 0.25) ** 2) / 0.012)
    sheen = min(1.0, sheen_a * 0.85 + sheen_b * 0.55)
    value = min(1.0, value + sheen * 0.55)
    sat = max(0.15, sat - sheen * 0.45)

    return hsv_to_rgb(hue, sat, value)


def add_box(verts, faces, x0, y0, x1, y1, z0, z1):
    base = len(verts)
    corners = [
        (x0, y0, z0),
        (x1, y0, z0),
        (x1, y1, z0),
        (x0, y1, z0),
        (x0, y0, z1),
        (x1, y0, z1),
        (x1, y1, z1),
        (x0, y1, z1),
    ]
    verts.extend(corners)
    quads = [
        (0, 1, 2, 3),
        (4, 7, 6, 5),
        (0, 4, 5, 1),
        (1, 5, 6, 2),
        (2, 6, 7, 3),
        (3, 7, 4, 0),
    ]
    for a, b, c, d in quads:
        faces.append((base + a, base + b, base + c, base + d))


def build_42_logo():
    verts = []
    faces = []
    depth = 0.35

    # Digit "4" — vertical stem, crossbar, left diagonal-ish block
    add_box(verts, faces, -1.55, -1.0, -1.15, 1.0, -depth, depth)  # right stem of 4
    add_box(verts, faces, -2.2, -0.15, -1.15, 0.25, -depth, depth)  # crossbar
    add_box(verts, faces, -2.2, -0.15, -1.8, 1.0, -depth, depth)  # left upper

    # Digit "2" — top bar, diagonal body as stepped boxes, bottom bar
    add_box(verts, faces, -0.55, 0.65, 0.75, 1.0, -depth, depth)  # top
    add_box(verts, faces, 0.35, 0.05, 0.75, 0.65, -depth, depth)  # right upper
    add_box(verts, faces, -0.55, -0.25, 0.75, 0.15, -depth, depth)  # mid
    add_box(verts, faces, -0.55, -1.0, -0.15, -0.25, -depth, depth)  # left lower
    add_box(verts, faces, -0.55, -1.0, 0.75, -0.65, -depth, depth)  # bottom

    # Small square accent (42 logo vibe)
    add_box(verts, faces, 0.95, -1.0, 1.35, -0.6, -depth * 0.6, depth * 0.6)

    return verts, faces


def write_obj(path: Path, verts, faces) -> None:
    lines = ["# scop 42 logo", "o FortyTwo"]
    for x, y, z in verts:
        lines.append(f"v {x:.6f} {y:.6f} {z:.6f}")
    # Simple planar UVs from XY
    xs = [v[0] for v in verts]
    ys = [v[1] for v in verts]
    min_x, max_x = min(xs), max(xs)
    min_y, max_y = min(ys), max(ys)
    dx = max(max_x - min_x, 1e-6)
    dy = max(max_y - min_y, 1e-6)
    for x, y, _ in verts:
        lines.append(f"vt {(x - min_x) / dx:.6f} {(y - min_y) / dy:.6f}")
    for a, b, c, d in faces:
        lines.append(
            f"f {a+1}/{a+1} {b+1}/{b+1} {c+1}/{c+1} {d+1}/{d+1}"
        )
    path.write_text("\n".join(lines) + "\n")


def build_cube():
    verts = [
        (-1, -1, -1),
        (1, -1, -1),
        (1, 1, -1),
        (-1, 1, -1),
        (-1, -1, 1),
        (1, -1, 1),
        (1, 1, 1),
        (-1, 1, 1),
    ]
    faces = [
        (0, 1, 2, 3),
        (4, 7, 6, 5),
        (0, 4, 5, 1),
        (1, 5, 6, 2),
        (2, 6, 7, 3),
        (3, 7, 4, 0),
    ]
    return verts, faces


def main() -> None:
    RES.mkdir(parents=True, exist_ok=True)
    write_bmp(RES / "unicorn.bmp", 512, 512, unicorn_color)
    write_obj(RES / "cube.obj", *build_cube())
    # Keep official curriculum models if present (42.obj / teapot*.obj from subject resources).
    official = RES / "42.obj"
    if not official.exists():
        write_obj(official, *build_42_logo())
        print("Generated fallback 42.obj (official resources missing)")
    print(f"Wrote assets in {RES}")


if __name__ == "__main__":
    main()
