#!/usr/bin/env python3
"""
Generate the split-sum GGX environment BRDF lookup table.

Usage:
    python Tools/GenerateBRDFLUT.py
    python Tools/GenerateBRDFLUT.py --size 128 --samples 1024 --output Graphics/BRDFLUT.h

The generated header contains one table as RG16F raw half-float bits and one
table as RG32F floats. Texture coordinates are x=roughness, y=NdotV.
"""

from __future__ import annotations

import argparse
import math
import struct
from pathlib import Path
from typing import Iterable


DEFAULT_SIZE = 128
DEFAULT_SAMPLES = 1024
DEFAULT_OUTPUT = Path(__file__).resolve().parents[1] / "Graphics" / "BRDFLUT.h"

Vector3 = tuple[float, float, float]


def integrate_brdf(roughness: float, ndotv: float, sample_count: int) -> tuple[float, float]:
    """Integrate the GGX split-sum scale/bias for one LUT texel."""
    ndotv = clamp(ndotv, 0.0, 1.0)
    roughness = clamp(roughness, 0.0, 1.0)

    view = (math.sqrt(max(0.0, 1.0 - ndotv * ndotv)), 0.0, ndotv)
    scale = 0.0
    bias = 0.0

    for sample_index in range(sample_count):
        xi = hammersley(sample_index, sample_count)
        half_vector = importance_sample_ggx(xi, roughness)

        vdoth = max(dot(view, half_vector), 0.0)
        light = normalize((
            2.0 * vdoth * half_vector[0] - view[0],
            2.0 * vdoth * half_vector[1] - view[1],
            2.0 * vdoth * half_vector[2] - view[2],
        ))

        ndotl = max(light[2], 0.0)
        ndoth = max(half_vector[2], 0.0)

        if ndotl <= 0.0 or ndoth <= 0.0 or vdoth <= 0.0:
            continue

        geometry = geometry_smith(ndotv, ndotl, roughness)
        visibility = geometry * vdoth / max(ndoth * ndotv, 1.0e-6)
        fresnel = pow(1.0 - vdoth, 5.0)

        scale += (1.0 - fresnel) * visibility
        bias += fresnel * visibility

    inv_sample_count = 1.0 / float(sample_count)
    return scale * inv_sample_count, bias * inv_sample_count


def clamp(value: float, minimum: float, maximum: float) -> float:
    return max(minimum, min(maximum, value))


def dot(a: Vector3, b: Vector3) -> float:
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]


def normalize(v: Vector3) -> Vector3:
    length_sq = dot(v, v)
    if length_sq <= 0.0:
        return (0.0, 0.0, 1.0)

    inv_length = 1.0 / math.sqrt(length_sq)
    return (v[0] * inv_length, v[1] * inv_length, v[2] * inv_length)


def radical_inverse_vdc(bits: int) -> float:
    bits = ((bits << 16) | (bits >> 16)) & 0xFFFFFFFF
    bits = (((bits & 0x55555555) << 1) | ((bits & 0xAAAAAAAA) >> 1)) & 0xFFFFFFFF
    bits = (((bits & 0x33333333) << 2) | ((bits & 0xCCCCCCCC) >> 2)) & 0xFFFFFFFF
    bits = (((bits & 0x0F0F0F0F) << 4) | ((bits & 0xF0F0F0F0) >> 4)) & 0xFFFFFFFF
    bits = (((bits & 0x00FF00FF) << 8) | ((bits & 0xFF00FF00) >> 8)) & 0xFFFFFFFF
    return float(bits) * 2.3283064365386963e-10


def hammersley(index: int, count: int) -> tuple[float, float]:
    return (float(index) / float(count), radical_inverse_vdc(index))


def importance_sample_ggx(xi: tuple[float, float], roughness: float) -> Vector3:
    alpha = roughness * roughness
    alpha_sq = alpha * alpha

    phi = 2.0 * math.pi * xi[0]
    cos_theta = math.sqrt((1.0 - xi[1]) / max(1.0 + (alpha_sq - 1.0) * xi[1], 1.0e-6))
    sin_theta = math.sqrt(max(0.0, 1.0 - cos_theta * cos_theta))

    return (
        math.cos(phi) * sin_theta,
        math.sin(phi) * sin_theta,
        cos_theta,
    )


def geometry_schlick_ggx(ndotx: float, roughness: float) -> float:
    k = (roughness * roughness) * 0.5
    return ndotx / max(ndotx * (1.0 - k) + k, 1.0e-6)


def geometry_smith(ndotv: float, ndotl: float, roughness: float) -> float:
    return geometry_schlick_ggx(ndotv, roughness) * geometry_schlick_ggx(ndotl, roughness)


def generate_lut(size: int, sample_count: int) -> list[tuple[float, float]]:
    values: list[tuple[float, float]] = []

    for y in range(size):
        ndotv = (float(y) + 0.5) / float(size)
        for x in range(size):
            roughness = (float(x) + 0.5) / float(size)
            values.append(integrate_brdf(roughness, ndotv, sample_count))

    return values


def float_to_half_bits(value: float) -> int:
    return struct.unpack("<H", struct.pack("<e", value))[0]


def format_float(value: float) -> str:
    if value == 0.0:
        return "0.0f"
    return f"{value:.9g}f"


def format_array(values: Iterable[str], values_per_line: int) -> str:
    lines: list[str] = []
    line_values: list[str] = []

    for value in values:
        line_values.append(value)
        if len(line_values) == values_per_line:
            lines.append("\t" + ", ".join(line_values) + ",")
            line_values = []

    if line_values:
        lines.append("\t" + ", ".join(line_values) + ",")

    return "\n".join(lines)


def write_header(path: Path, values: list[tuple[float, float]], size: int, sample_count: int) -> None:
    half_values: list[str] = []
    float_values: list[str] = []

    for scale, bias in values:
        half_values.append(f"0x{float_to_half_bits(scale):04X}")
        half_values.append(f"0x{float_to_half_bits(bias):04X}")
        float_values.append(format_float(scale))
        float_values.append(format_float(bias))

    header = f"""#pragma once

// Generated by Tools/GenerateBRDFLUT.py.
// Split-sum GGX environment BRDF LUT. Coordinates: x=roughness, y=NdotV.

#include <cstdint>

namespace BRDFLUT
{{

static constexpr int BRDF_LUT_SIZE = {size};
static constexpr int BRDF_LUT_SAMPLE_COUNT = {sample_count};
static constexpr int BRDF_LUT_COMPONENTS = 2;
static constexpr float BRDF_LUT_SCALE = (float)(BRDF_LUT_SIZE - 1) / (float)BRDF_LUT_SIZE;
static constexpr float BRDF_LUT_BIAS = 0.5f / (float)BRDF_LUT_SIZE;

static const std::uint16_t BRDF_LUT_RG16F[BRDF_LUT_SIZE * BRDF_LUT_SIZE * BRDF_LUT_COMPONENTS] =
{{
{format_array(half_values, 12)}
}};

static const float BRDF_LUT_RG32F[BRDF_LUT_SIZE * BRDF_LUT_SIZE * BRDF_LUT_COMPONENTS] =
{{
{format_array(float_values, 8)}
}};

}}
"""

    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(header, encoding="utf-8", newline="\n")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Generate the split-sum GGX BRDF LUT header.")
    parser.add_argument("--size", type=int, default=DEFAULT_SIZE, help="LUT width and height in texels.")
    parser.add_argument("--samples", type=int, default=DEFAULT_SAMPLES, help="Hammersley samples per texel.")
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT, help="Generated header path.")
    return parser.parse_args()


def main() -> int:
    args = parse_args()

    if args.size <= 0:
        raise ValueError("--size must be positive")
    if args.samples <= 0:
        raise ValueError("--samples must be positive")

    values = generate_lut(args.size, args.samples)
    write_header(args.output, values, args.size, args.samples)
    print(f"Wrote {args.output} ({args.size}x{args.size}, {args.samples} samples)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
