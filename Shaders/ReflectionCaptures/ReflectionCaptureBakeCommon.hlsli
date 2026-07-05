#pragma once

// Cube face direction mapping and sampling sequences shared by the reflection capture
// bake and convolve shaders. Face/UV convention matches hardware cubemap addressing
// (+X, -X, +Y, -Y, +Z, -Z), so texels written per-face here can be read back with a
// regular TextureCube sample in the same direction.

float3 ReflectionCaptureCubeFaceDirectionFromUV(uint Face, float2 UV)
{
	float3 Direction = float3(0, 0, 1);
	if (Face == 0u) Direction = float3( 1.0, -UV.y, -UV.x);
	if (Face == 1u) Direction = float3(-1.0, -UV.y,  UV.x);
	if (Face == 2u) Direction = float3( UV.x,  1.0,  UV.y);
	if (Face == 3u) Direction = float3( UV.x, -1.0, -UV.y);
	if (Face == 4u) Direction = float3( UV.x, -UV.y,  1.0);
	if (Face == 5u) Direction = float3(-UV.x, -UV.y, -1.0);
	return normalize(Direction);
}

float3 ReflectionCaptureCubeFaceDirection(uint Face, uint2 Pixel, uint FaceSize)
{
	float2 UV = ((float2(Pixel) + 0.5) / float(FaceSize)) * 2.0 - 1.0;
	return ReflectionCaptureCubeFaceDirectionFromUV(Face, UV);
}

float ReflectionCaptureRadicalInverseVdC(uint Bits)
{
	Bits = (Bits << 16u) | (Bits >> 16u);
	Bits = ((Bits & 0x55555555u) << 1u) | ((Bits & 0xAAAAAAAAu) >> 1u);
	Bits = ((Bits & 0x33333333u) << 2u) | ((Bits & 0xCCCCCCCCu) >> 2u);
	Bits = ((Bits & 0x0F0F0F0Fu) << 4u) | ((Bits & 0xF0F0F0F0u) >> 4u);
	Bits = ((Bits & 0x00FF00FFu) << 8u) | ((Bits & 0xFF00FF00u) >> 8u);
	return float(Bits) * 2.3283064365386963e-10;
}

float2 ReflectionCaptureHammersley(uint Index, uint Count)
{
	return float2((float(Index) + 0.5) / float(max(Count, 1u)), ReflectionCaptureRadicalInverseVdC(Index));
}

float2 ReflectionCaptureRotatedSample(uint TexelIndex, uint SampleIndex, uint SampleCount)
{
	uint Seed = Random::Hash(TexelIndex);
	float2 Rotation = float2(Random::FloatConstruct(Seed), Random::FloatConstruct(Random::Hash(Seed)));
	return clamp(frac(ReflectionCaptureHammersley(SampleIndex, SampleCount) + Rotation), 0.0001, 0.9999);
}
