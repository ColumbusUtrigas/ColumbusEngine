#pragma pack_matrix(row_major)

#include "Common.hlsli"

#define GPU_SCENE_NO_BINDINGS
#include "GPUScene.hlsli"

struct VS_TO_PS
{
	float4 Pos : SV_POSITION;
};

// TODO: move away from big push constants, use ID + cbuffer
[[vk::push_constant]]
struct _Params {
	float4x4 Model;
	float4x4 ModelInverse;
	float4x4 VP;
} Params;

[[vk::binding(0, 0)]] Texture2D<float> GBufferDepth;
[[vk::binding(1, 0)]] StructuredBuffer<GPUSceneStruct> GPUSceneScene;

[[vk::binding(0, 1)]] Texture2D<float4> Texture;
[[vk::binding(1, 1)]] SamplerState Sampler;

static const float3 pos[8] = {
	float3(-1, -1, -1),
	float3(+1, -1, -1),
	float3(+1, -1, +1),
	float3(-1, -1, +1),

	float3(-1, +1, -1),
	float3(+1, +1, -1),
	float3(+1, +1, +1),
	float3(-1, +1, +1),
};

static const int index[36] = {
	0, 1, 2,
	0, 2, 3,

	4, 5, 6,
	4, 6, 7,

	0, 4, 7,
	0, 3, 7,

	1, 5, 6,
	1, 2, 6,

	1, 5, 4,
	1, 0, 4,

	2, 6, 7,
	2, 3, 7,
};

VS_TO_PS Vertex(uint VertexID : SV_VertexID)
{
	VS_TO_PS Out;
    float4 World = mul(Params.Model, float4(pos[index[VertexID]] * 0.5f, 1));
    Out.Pos = mul(Params.VP, World) * float4(1, -1, 1, 1);
	return Out;
}

float4 Pixel(VS_TO_PS In) : SV_TARGET
{
	uint2 Pixel = uint2(In.Pos.xy);
	float Depth = GBufferDepth[Pixel];
	clip(Depth - DEVICE_DEPTH_SKY_EPSILON);
	float4 WorldPos = float4(ReconstructWorldPositionFromDepth(Pixel, Depth, GPUSceneScene[0].RenderSize, GPUSceneScene[0].CameraCur.InverseViewProjectionMatrix), 1);

    float4 ObjectPos = mul(Params.ModelInverse, WorldPos);
	clip(0.5 - abs(ObjectPos.xyz));
	float2 UV = ObjectPos.xz + 0.5; // why do I have to use xz exactly?
	// return float4(UV, 0, 0);

	return Texture.Sample(Sampler, UV);
}
