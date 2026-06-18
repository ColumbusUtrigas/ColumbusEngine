#define SET 0
#include "../RayTracingIrradianceVolumes.hlsli"
#include "../Common.hlsli"

#define GPU_SCENE_NO_BINDINGS
#include "../GPUScene.hlsli"

[[vk::binding(0, 1)]] Texture2D<float> GBufferDepth;
[[vk::binding(1, 1)]] Texture2D<float2> GBufferNormal;
[[vk::binding(2, 1)]] StructuredBuffer<GPUSceneStruct> GPUSceneScene;
[[vk::binding(3, 1)]] RWTexture2D<half4> Output;

[[vk::push_constant]]
struct _Params
{
    uint2 Resolution;
    uint2 Padding;
} Params;

[numthreads(8, 8, 1)]
void main(uint3 dtid : SV_DispatchThreadID)
{
    if (any(dtid.xy >= Params.Resolution))
        return;
    
	float Depth = GBufferDepth[dtid.xy];
	if (IsSkyDepth(Depth))
	{
		Output[dtid.xy] = float4(0, 0, 0, 1);
		return;
	}

    float3 WP = ReconstructWorldPositionFromDepth(dtid.xy, Depth, GPUSceneScene[0].RenderSize, GPUSceneScene[0].CameraCur.InverseViewProjectionMatrix);
    float3 Normal = NormalDecode(GBufferNormal[dtid.xy]);
    float3 Irradiance = SampleRuntimeIrradianceVolumes(WP, Normal);
    Output[dtid.xy] = float4(Irradiance, 1);
}
