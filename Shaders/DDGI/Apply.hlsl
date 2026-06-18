#include "Defines.h"
#include "../Common.hlsli"

#define GPU_SCENE_NO_BINDINGS
#include "../GPUScene.hlsli"

#include "SDK/ddgi/Irradiance.hlsl"
//#include "SDK/ddgi/include/DDGIRootConstants.hlsl"

[[vk::binding(0, 0)]] Texture2D<float> GBufferDepth;
[[vk::binding(1, 0)]] Texture2D<float2> GBufferNormal;
[[vk::binding(2, 0)]] StructuredBuffer<GPUSceneStruct> GPUSceneScene;
[[vk::binding(3, 0)]] StructuredBuffer<DDGIVolumeDescGPUPacked> DDGIVolumes;
[[vk::binding(4, 0)]] Texture2DArray<float4> ProbeData;
[[vk::binding(5, 0)]] Texture2DArray<float4> ProbeIrradiance;
[[vk::binding(6, 0)]] Texture2DArray<float4> ProbeDistance;
[[vk::binding(7, 0)]] SamplerState BilinearSampler;
[[vk::binding(8, 0)]] RWTexture2D<float4> Output;
// TODO: multiple volumes

[[vk::push_constant]]
struct _Params
{
	uint2  Resolution;
	uint2  _Padding;
} Params;

[numthreads(8, 8, 1)]
void main(uint3 dtid : SV_DispatchThreadID)
{
    if (any(dtid.xy >= Params.Resolution))
		return;
			
	// Get the DDGIVolume's index (from root/push constants)
	uint volumeIndex = 0;

	// Get the DDGIVolume's constants from the structured buffer
	DDGIVolumeDescGPU volume = UnpackDDGIVolumeDescGPU(DDGIVolumes[volumeIndex]);
	
	// Get the volume resources needed for the irradiance query
	DDGIVolumeResources resources;
	resources.probeIrradiance = ProbeIrradiance;
	resources.probeDistance = ProbeDistance;
	resources.probeData = ProbeData;
	resources.bilinearSampler = BilinearSampler;
	
	float depth = GBufferDepth[dtid.xy];
	if (IsSkyDepth(depth))
	{
		Output[dtid.xy] = float4(0, 0, 0, 1);
		return;
	}

    float3 worldPosition = ReconstructWorldPositionFromDepth(dtid.xy, depth, GPUSceneScene[0].RenderSize, GPUSceneScene[0].CameraCur.InverseViewProjectionMatrix);
    float3 normal = NormalDecode(GBufferNormal[dtid.xy]);

	// Compute volume blending weight
    float volumeBlendWeight = DDGIGetVolumeBlendWeight(worldPosition, volume);
			
	float3 irradiance = float3(0, 0, 0);
	
    float3 cameraDirection = normalize(worldPosition.xyz - GPUSceneScene[0].CameraCur.CameraPosition.xyz);
    float3 surfaceBias = DDGIGetSurfaceBias(normal, cameraDirection, volume);

	// Don't evaluate irradiance when the surface is outside the volume
	if (volumeBlendWeight > 0)
	{
		// Get irradiance from the DDGIVolume
		irradiance = DDGIGetVolumeIrradiance(
			worldPosition,
			surfaceBias,
			normal,
			volume,
			resources);

		// Attenuate irradiance by the blend weight
		irradiance *= volumeBlendWeight;
	}
	
    Output[dtid.xy] = float4(irradiance, 1);
	//Output[dtid.xy] = float4(volumeBlendWeight, volumeBlendWeight, volumeBlendWeight, 1);
}
