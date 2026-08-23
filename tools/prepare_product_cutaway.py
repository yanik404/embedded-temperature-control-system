#!/usr/bin/env python3
"""Prepare the approved interior render for the embedded dashboard.

The image-generation pass establishes coherent materials and lighting.  This
post-processing step keeps the delivered PCB artwork exact, removes the baked
checkerboard non-destructively and scales the result to the dashboard's fixed
702 x 1100 product viewport.
"""

from __future__ import annotations

import argparse
from collections import deque
from pathlib import Path

from PIL import Image, ImageChops, ImageDraw, ImageEnhance, ImageFilter


CANVAS_SIZE = (702, 1100)


def checkerboard_alpha(image: Image.Image) -> Image.Image:
    """Make only the bright neutral background connected to an edge transparent."""

    rgb = image.convert("RGB")
    width, height = rgb.size
    pixels = rgb.load()
    visited = bytearray(width * height)
    queue: deque[tuple[int, int]] = deque()

    def is_background(x: int, y: int) -> bool:
        red, green, blue = pixels[x, y]
        return min(red, green, blue) >= 224 and max(red, green, blue) - min(red, green, blue) <= 8

    def add(x: int, y: int) -> None:
        offset = y * width + x
        if not visited[offset] and is_background(x, y):
            visited[offset] = 1
            queue.append((x, y))

    for x in range(width):
        add(x, 0)
        add(x, height - 1)
    for y in range(height):
        add(0, y)
        add(width - 1, y)

    while queue:
        x, y = queue.popleft()
        if x:
            add(x - 1, y)
        if x + 1 < width:
            add(x + 1, y)
        if y:
            add(x, y - 1)
        if y + 1 < height:
            add(x, y + 1)

    alpha = Image.new("L", (width, height), 255)
    alpha_pixels = alpha.load()
    for y in range(height):
        row = y * width
        for x in range(width):
            if visited[row + x]:
                alpha_pixels[x, y] = 0

    # A small feather removes checker-colored antialiasing without softening the product.
    alpha = alpha.filter(ImageFilter.GaussianBlur(0.45))
    result = image.convert("RGBA")
    result.putalpha(alpha)
    return result


def exact_pcb_layer(reference: Image.Image, target_size: tuple[int, int]) -> Image.Image:
    """Extract the circular board from the supplied top render and foreshorten it."""

    # The bounds are measured from PCB_TOP_original_3D.png, not generated artwork.
    crop_box = (198, 111, 913, 826)
    board = reference.convert("RGBA").crop(crop_box)
    source_mask = Image.new("L", board.size, 0)
    ImageDraw.Draw(source_mask).ellipse((2, 2, board.width - 3, board.height - 3), fill=255)
    source_mask = source_mask.filter(ImageFilter.GaussianBlur(1.2))
    board.putalpha(source_mask)
    board = board.resize(target_size, Image.Resampling.LANCZOS)

    # Match the restrained light level inside the graphite housing.
    board_rgb = ImageEnhance.Brightness(board.convert("RGB")).enhance(0.82)
    board_rgb = ImageEnhance.Contrast(board_rgb).enhance(1.04)
    board = Image.merge("RGBA", (*board_rgb.split(), board.getchannel("A")))

    shade = Image.new("L", target_size)
    shade_pixels = shade.load()
    for y in range(target_size[1]):
        level = int(230 - 32 * y / max(1, target_size[1] - 1))
        for x in range(target_size[0]):
            shade_pixels[x, y] = level
    board.putalpha(ImageChops.multiply(board.getchannel("A"), shade))
    return board


def composite_original_pcb(product: Image.Image, reference: Image.Image) -> Image.Image:
    """Place the exact board beneath the already-rendered Pico, fan and mounts."""

    # Placement in the approved clear engineering cutaway source.
    board_box = (422, 873, 912, 1138)
    board = exact_pcb_layer(reference, (board_box[2] - board_box[0], board_box[3] - board_box[1]))

    # The generated Pico W and physical occluders remain above the exact PCB texture.
    mask = board.getchannel("A")
    draw = ImageDraw.Draw(mask)
    draw.polygon(((34, 55), (290, 64), (286, 226), (28, 220)), fill=0)
    draw.rounded_rectangle((20, 0, 407, 88), radius=28, fill=0)  # fan frame
    for center in ((14, 20), (467, 20), (13, 216), (440, 231)):
        x, y = center
        draw.ellipse((x - 18, y - 18, x + 18, y + 18), fill=0)
    board.putalpha(mask.filter(ImageFilter.GaussianBlur(0.65)))

    result = product.copy()
    result.alpha_composite(board, (board_box[0], board_box[1]))
    return result


def fit_canvas(image: Image.Image) -> Image.Image:
    bounds = image.getchannel("A").getbbox()
    if bounds:
        padding = 8
        bounds = (
            max(0, bounds[0] - padding),
            max(0, bounds[1] - padding),
            min(image.width, bounds[2] + padding),
            min(image.height, bounds[3] + padding),
        )
        image = image.crop(bounds)
    ratio = min(CANVAS_SIZE[0] / image.width, CANVAS_SIZE[1] / image.height)
    resized = image.resize((round(image.width * ratio), round(image.height * ratio)), Image.Resampling.LANCZOS)
    canvas = Image.new("RGBA", CANVAS_SIZE, (0, 0, 0, 0))
    canvas.alpha_composite(resized, ((CANVAS_SIZE[0] - resized.width) // 2, (CANVAS_SIZE[1] - resized.height) // 2))
    return canvas


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("source", type=Path)
    parser.add_argument("pcb_reference", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()

    product = checkerboard_alpha(Image.open(args.source))
    product = composite_original_pcb(product, Image.open(args.pcb_reference))
    product = fit_canvas(product)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    product.save(args.output, "WEBP", quality=84, method=6, exact=True)


if __name__ == "__main__":
    main()
