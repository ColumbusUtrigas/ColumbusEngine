/**********************************************************************
Copyright (c) 2021 Advanced Micro Devices, Inc. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
********************************************************************/

#ifndef FFX_DNSR_SHADOWS_PREPARESHADOWMASK_HLSL
#define FFX_DNSR_SHADOWS_PREPARESHADOWMASK_HLSL

#include "DenoiserShadowsUtil.hlsli"

// user code

[[vk::push_constant]]
struct _Params {
	int2 BufferDimensions;
	int2 PackedBufferDimensions;
} Params;

[[vk::image_format("r8")]]    RWTexture2D<float> InputBuffer : register(u0);
[[vk::image_format("r32ui")]] RWTexture2D<uint>  PackedOutputBuffer : register(u1);

#if 1
groupshared uint collected[8][4];

[numthreads(8, 4, 1)]
void main(uint3 gtid : SV_GroupThreadID, uint3 gid : SV_GroupID, uint3 did : SV_DispatchThreadID)
{
	uint2 pixel = gid.xy * uint2(8, 4) + gtid.xy;
	const bool hit_light = InputBuffer[pixel] > 0.01; // true if not shadow
	const uint lane_mask = hit_light ? FFX_DNSR_Shadows_GetBitMaskFromPixelPosition(pixel) : 0;
	collected[gtid.x][gtid.y] = lane_mask;
	GroupMemoryBarrierWithGroupSync();
	const uint mask = WaveActiveBitOr(lane_mask); // TODO: why doesn't it work?

	// if (gtid.x == 0 && gtid.y == 0) // only first thread in the group writes to global memory
	// TODO: use wave intrinsics instead of this mess
	if (gtid.x == 0 && gtid.y == 0)
	{
		uint result = 0;
		for (int i = 0; i < 8; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				result |= collected[i][j];
			}
		}

		PackedOutputBuffer[gid.xy] = result;
	}
}
#endif

#if 0

uint2 FFX_DNSR_Shadows_GetBufferDimensions()
{
	return Params.BufferDimensions;
}

bool FFX_DNSR_Shadows_HitsLight(uint2 did, uint2 gtid, uint2 gid)
{
	// did is a pixel position?
	return InputBuffer[did] > 0.01;
}

void FFX_DNSR_Shadows_WriteMask(uint linear_tile_index, uint mask)
{
	uint width = Params.PackedBufferDimensions.x;
	uint2 coords = uint2(linear_tile_index % width, (linear_tile_index + width - 1) / width);
	PackedOutputBuffer[coords] = mask;
}

// end user code

void FFX_DNSR_Shadows_CopyResult(uint2 gtid, uint2 gid)
{
	const uint2 did = gid * uint2(8, 4) + gtid;
	const uint linear_tile_index = FFX_DNSR_Shadows_LinearTileIndex(gid, FFX_DNSR_Shadows_GetBufferDimensions().x);
	const bool hit_light = FFX_DNSR_Shadows_HitsLight(did, gtid, gid);
	const uint lane_mask = hit_light ? FFX_DNSR_Shadows_GetBitMaskFromPixelPosition(did) : 0;
	FFX_DNSR_Shadows_WriteMask(linear_tile_index, WaveActiveBitOr(lane_mask));
}

void FFX_DNSR_Shadows_PrepareShadowMask(uint2 gtid, uint2 gid)
{
	gid *= 4;
	uint2 tile_dimensions = (FFX_DNSR_Shadows_GetBufferDimensions() + uint2(7, 3)) / uint2(8, 4);

	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			uint2 tile_id = uint2(gid.x + i, gid.y + j);
			tile_id = clamp(tile_id, 0, tile_dimensions - 1);
			FFX_DNSR_Shadows_CopyResult(gtid, tile_id);
		}
	}
}

[numthreads(8, 4, 1)]
void main(uint3 gtid : SV_GroupThreadID, uint3 gid : SV_GroupID)
{
	FFX_DNSR_Shadows_PrepareShadowMask(gtid.xy, gid.xy);
}
#endif

#endif