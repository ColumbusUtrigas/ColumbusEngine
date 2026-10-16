#define GPU_SCENE_NO_BINDINGS
#include "GPUScene.hlsli"
#include "TiledLightsCommon.hlsli"

[[vk::binding(0, 0)]] Texture2D<float> Depth;
[[vk::binding(1, 0)]] StructuredBuffer<GPULight> Lights;
[[vk::binding(2, 0)]] StructuredBuffer<GPUSceneStruct> Scene;
[[vk::binding(3, 0)]] RWStructuredBuffer<uint> Tiles;

static const uint LightsPerGroup = LightTileSize * LightTileSize;
static const uint WordsPerGroup  = LightsPerGroup / LightTileBits;

groupshared uint MinDepth;
groupshared uint MaxDepth;
groupshared uint LightMasks[WordsPerGroup];

groupshared float3 TileCenter;
groupshared float3 TileExtent;
groupshared float4 TilePlanes[4];

float4 NormalizePlane(float4 Plane)
{
	return Plane * rsqrt(dot(Plane.xyz, Plane.xyz));
}

void PrepareTileBounds(uint2 TileId)
{
	const uint2 RenderSize = Scene[0].RenderSize;
	const uint2 PixelMin   = TileId * LightTileSize;
	const uint2 PixelMax   = min(PixelMin + LightTileSize, RenderSize);
	const float2 NdcMin    = ScreenUVToNDC(float2(PixelMin) / RenderSize);
	const float2 NdcMax    = ScreenUVToNDC(float2(PixelMax) / RenderSize);

	float3 BoundsMin = float3(1e30, 1e30, 1e30);
	float3 BoundsMax = -BoundsMin;

	[unroll]
	for (uint i = 0; i < 8; ++i)
	{
		const float2 Ndc        = float2((i & 1) ? NdcMax.x : NdcMin.x, (i & 2) ? NdcMax.y : NdcMin.y);
		const float  DepthValue = asfloat((i & 4) ? MaxDepth : MinDepth);
		const float3 Position   = ReconstructViewPositionFromDepthNDC(Ndc, DepthValue, Scene[0].CameraCur.InverseProjectionMatrix);
		BoundsMin = min(BoundsMin, Position);
		BoundsMax = max(BoundsMax, Position);
	}

	TileCenter = (BoundsMin + BoundsMax) * 0.5;
	TileExtent = (BoundsMax - BoundsMin) * 0.5 + 0.001;

	const float4x4 Projection = Scene[0].CameraCur.ProjectionMatrix;
	TilePlanes[0] = NormalizePlane(Projection[0] - NdcMin.x * Projection[3]);
	TilePlanes[1] = NormalizePlane(NdcMax.x * Projection[3] - Projection[0]);
	TilePlanes[2] = NormalizePlane(Projection[1] - NdcMax.y * Projection[3]);
	TilePlanes[3] = NormalizePlane(NdcMin.y * Projection[3] - Projection[1]);
}

bool LightIntersectsTile(GPULight Light)
{
	if (Light.Type == GPULIGHT_DIRECTIONAL)
	{
		return true;
	}

	const float3 Position  = mul(Scene[0].CameraCur.ViewMatrix, float4(Light.Position.xyz, 1)).xyz;
	const float3 Direction = mul((float3x3)Scene[0].CameraCur.ViewMatrix, Light.Direction.xyz);

	if (Light.Type == GPULIGHT_RECTANGLE || Light.Type == GPULIGHT_DISC)
	{
		// LTC has no finite range cutoff. Only the non-emitting half-space can be rejected.
		if ((Light.Flags & GPULIGHT_FLAG_TWOSIDED) != 0)
		{
			return true;
		}

		return dot(Direction, TileCenter - Position) + dot(abs(Direction), TileExtent) >= -0.001;
	}

	if (Light.Type != GPULIGHT_POINT && Light.Type != GPULIGHT_SPOT)
	{
		return false; // Deferred lighting does not shade other light types yet.
	}

	if (Light.Range <= 0)
	{
		return false;
	}

	const float  Radius     = Light.Range + max(0.001, abs(Position.z) * 1e-5);
	const float3 Separation = max(abs(Position - TileCenter) - TileExtent, 0);

	if (dot(Separation, Separation) > Radius * Radius)
	{
		return false;
	}

	[unroll]
	for (uint i = 0; i < 4; ++i)
	{
		if (dot(TilePlanes[i].xyz, Position) + TilePlanes[i].w < -Radius)
		{
			return false;
		}
	}

	if (Light.Type == GPULIGHT_SPOT && Light.SizeOrSpotAngles.y > 0)
	{
		const float3 ToTile = TileCenter - Position;
		const float3 Axis   = -normalize(Direction);
		const float  Axial  = dot(ToTile, Axis);
		const float  Radial = length(ToTile - Axis * Axial);
		const float  Cosine = Light.SizeOrSpotAngles.y;
		const float  Sine   = sqrt(saturate(1 - Cosine * Cosine));

		const float TileRadius = length(TileExtent);

		// A sphere enclosing the tile makes both cone tests conservative.
		if (Axial + TileRadius < 0 || Cosine * Radial - Sine * Axial > TileRadius)
		{
			return false;
		}
	}

	return true;
}

[numthreads(LightTileSize, LightTileSize, 1)]
void main(uint3 GroupId : SV_GroupID, uint3 ThreadId : SV_DispatchThreadID, uint GroupIndex : SV_GroupIndex)
{
	const uint WordsPerTile = GetLightTileWordCount(Scene[0].LightsCount);
	const uint TileIndex    = GroupId.x + GroupId.y * GetLightTileCount(Scene[0].RenderSize).x;
	const uint TileOffset   = TileIndex * WordsPerTile;

	if (GroupIndex == 0)
	{
		MinDepth = asuint(1.0);
		MaxDepth = 0;
	}

	GroupMemoryBarrierWithGroupSync();

	if (all(ThreadId.xy < uint2(Scene[0].RenderSize)))
	{
		const float DepthValue = Depth[ThreadId.xy];

		if (IsSceneDepth(DepthValue))
		{
			InterlockedMin(MinDepth, asuint(DepthValue));
			InterlockedMax(MaxDepth, asuint(DepthValue));
		}
	}

	GroupMemoryBarrierWithGroupSync();

	// This branch is uniform across the group. Clear every word, including empty scenes.
	if (MaxDepth == 0 || Scene[0].LightsCount == 0)
	{
		for (uint i = GroupIndex; i < WordsPerTile; i += LightsPerGroup)
		{
			Tiles[TileOffset + i] = 0;
		}

		return;
	}

	if (GroupIndex == 0)
	{
		PrepareTileBounds(GroupId.xy);
	}

	GroupMemoryBarrierWithGroupSync();

	// Process batches instead of imposing a maximum number of lights per tile.
	for (uint FirstWord = 0; FirstWord < WordsPerTile; FirstWord += WordsPerGroup)
	{
		if (GroupIndex < WordsPerGroup)
		{
			LightMasks[GroupIndex] = 0;
		}

		GroupMemoryBarrierWithGroupSync();

		const uint LightId = FirstWord * LightTileBits + GroupIndex;

		if (LightId < Scene[0].LightsCount)
		{
			if (LightIntersectsTile(Lights[LightId]))
			{
				InterlockedOr(LightMasks[GroupIndex / LightTileBits], 1u << (GroupIndex % LightTileBits));
			}
		}

		GroupMemoryBarrierWithGroupSync();

		if (GroupIndex < WordsPerGroup && FirstWord + GroupIndex < WordsPerTile)
		{
			Tiles[TileOffset + FirstWord + GroupIndex] = LightMasks[GroupIndex];
		}

		GroupMemoryBarrierWithGroupSync();
	}
}
