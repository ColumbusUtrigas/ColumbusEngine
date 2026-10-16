struct VS_TO_PS
{
	float4 Pos : SV_POSITION;
};

#ifdef VERTEX_SHADER
static const float2 pos[3] =
{
	float2(-1, -1),
	float2(-1, 3),
	float2(3, -1),
};

VS_TO_PS Vertex(uint VertexID : SV_VertexID)
{
	VS_TO_PS Out;
	Out.Pos = float4(pos[VertexID], 0, 1);
	return Out;
}
#endif

#ifdef PIXEL_SHADER
#include "Common.hlsli"

[[vk::push_constant]]
struct _Params
{
	uint2 ViewportSize;
	uint Flags;
} Params;

#define FLAG_R_ONLY 1
#define FLAG_G_ONLY 2
#define FLAG_NORMAL 4
#define FLAG_DEPTH  8

#if TILED_LIGHTS
#define GPU_SCENE_NO_BINDINGS
#include "GPUScene.hlsli"
#include "TiledLightsCommon.hlsli"
#include "ShaderDebugText.hlsli"

[[vk::binding(2, 0)]] StructuredBuffer<uint> LightTiles;
[[vk::binding(3, 0)]] StructuredBuffer<GPUSceneStruct> Scene;

float3 LightCountHeatmap(uint Count)
{
	if (Count == 0)
	{
		return float3(0.015, 0.02, 0.04);
	}

	// Logarithmic blue/cyan/green/yellow/red ramp; 64 lights reaches the red end.
	const float T = saturate(log2(float(Count) + 1) / log2(65.0));
	return saturate(1.5 - abs(4 * T - float3(3, 2, 1)));
}

float4 VisualiseTiledLights(uint2 Pixel)
{
	const uint2 RenderSize  = Params.ViewportSize;
	const uint2 Tile        = Pixel / LightTileSize;
	const uint  WordCount   = GetLightTileWordCount(Scene[0].LightsCount);
	const uint  TileOffset  = (Tile.x + Tile.y * GetLightTileCount(RenderSize).x) * WordCount;
	uint LightCount = 0;

	for (uint i = 0; i < WordCount; ++i)
	{
		LightCount += countbits(LightTiles[TileOffset + i]);
	}

	float3 Colour = LightCountHeatmap(LightCount);
	const uint2 TileOrigin = Tile * LightTileSize;
	const uint2 TileSize   = min(LightTileSize, RenderSize - TileOrigin);

	if (any(Pixel == TileOrigin))
	{
		Colour *= 0.55;
	}

	// Tiny partial tiles cannot hold a legible glyph, but still show their heat value.
	if (all(TileSize >= 3))
	{
		ShaderDebugString Text = DebugTextEmpty();
		DebugTextAppendUInt(Text, LightCount);

		const float2 TextSize = float2(Text.Length * 5 - 1, 7);
		const float2 Scale    = min(1.0, (float2(TileSize) - 2) / TextSize);
		const float2 Origin   = floor(float2(TileOrigin) + (float2(TileSize) - TextSize * Scale) * 0.5);
		const float2 Position = float2(Pixel) + 0.5;

		if (all(Position >= Origin - 1) && all(Position < Origin + TextSize * Scale + 1))
		{
			Colour *= 0.3;
		}

		const float Coverage = DebugTextDrawString(Text, Position, Origin, Scale, float2(5, 12));
		Colour = lerp(Colour, 1.0, Coverage);
	}

	return float4(Colour, 1);
}
#else
[[vk::binding(0, 0)]] Texture2D InputTexture;
[[vk::binding(1, 0)]] SamplerState LinearSampler;
#endif

float4 Pixel(VS_TO_PS In, float4 Coord : SV_Position) : SV_TARGET
{
#if TILED_LIGHTS
	return VisualiseTiledLights(uint2(Coord.xy));
#else
	float2 UV = Coord.xy / float2(Params.ViewportSize);

	float4 SourceColour = float4(InputTexture.Sample(LinearSampler, UV).rgb, 1);

	if (Params.Flags == FLAG_R_ONLY)
	{
		return float4(SourceColour.r, SourceColour.r, SourceColour.r, 1);
	}
	else if (Params.Flags == FLAG_G_ONLY)
	{
		return float4(SourceColour.g, SourceColour.g, SourceColour.g, 1);
	}
	else if (Params.Flags == FLAG_NORMAL)
	{
		return float4(NormalDecode(SourceColour.rg) * 0.5 + 0.5, 1);
	}
	else if (Params.Flags == FLAG_DEPTH)
	{
		return float4(SourceColour.rrr, 1);
	}

	return SourceColour;
#endif
}
#endif
