#include "GPUScene.hlsli"

[[vk::push_constant]]
struct ShadowDrawConstants
{
	float4x4 WorldToClip;
	uint     ObjectId;
} Parameters;

struct VertexOutput
{
	float4 Position : SV_POSITION;
#if ALPHA_MASKED
	float2               UV         : TEXCOORD0;
	nointerpolation uint MaterialId : COLOR0;
#endif
};

VertexOutput VSMain(uint VertexId : SV_VertexID)
{
	GPUSceneMeshCompact Mesh     = GPUScene::GPUSceneMeshes[Parameters.ObjectId];
	uint                Index    = GPUScene::FetchIndex(Parameters.ObjectId, VertexId);
	float3              Position = GPUScene::FetchVertexPositionFromMesh(Mesh, Index);
	VertexOutput        Output;
	Output.Position = mul(Parameters.WorldToClip, mul(Mesh.Transform, float4(Position, 1))) * float4(1, -1, 1, 1);
#if ALPHA_MASKED
	Output.UV         = GPUScene::FetchVertexUVFromMesh(Mesh, Index);
	Output.MaterialId = Mesh.MaterialId;
#endif
	return Output;
}

#if ALPHA_MASKED
void PSMain(VertexOutput Input)
{
	GPUSceneMaterialCompact Material = GPUScene::GPUSceneMaterials[Input.MaterialId];
	clip(GPUScene::SampleMaterialAlpha(Material, Input.UV) - Material.AlphaCutoff);
}
#endif
