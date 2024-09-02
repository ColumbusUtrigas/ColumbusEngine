#include "BRDF.hlsli"

// we need only definitions from GPUScene
#define GPU_SCENE_NO_BINDINGS
#include "GPUScene.hlsli"

[[vk::binding(0, 0)]] Texture2D GBufferAlbedo;
[[vk::binding(1, 0)]] Texture2D GBufferNormal;
[[vk::binding(2, 0)]] Texture2D GBufferWorldPosition;
[[vk::binding(3, 0)]] Texture2D GBufferRoughnessMetallic;
[[vk::binding(4, 0)]] Texture2D GBufferLightmap; // TODO: unify with GI?
[[vk::binding(5, 0)]] Texture2D GBufferReflections;
[[vk::binding(6, 0)]] Texture2D GBufferGI;
[[vk::binding(7, 0)]] RWTexture2D<float4> LightingOutput;

[[vk::binding(8, 0)]] StructuredBuffer<GPULight> GPUSceneLights;
[[vk::binding(9, 0)]] StructuredBuffer<GPUSceneStruct> GPUSceneScene;

[[vk::binding(0, 1)]] RWTexture2D<float> ShadowTextures[1000];

[numthreads(8, 8, 1)]
void main(uint3 dtid : SV_DispatchThreadID)
{
	const uint2 Pixel = dtid.xy;

    if (any(Pixel >= GPUSceneScene[0].RenderSize))
		return;

	const float3 WP = GBufferWorldPosition[Pixel].rgb;
	const float2 RM = GBufferRoughnessMetallic[Pixel].rg;

	BRDFData BRDF;
	BRDF.N         = GBufferNormal[Pixel].rgb;
	BRDF.V         = -normalize(WP - GPUSceneScene[0].CameraCur.CameraPosition.xyz);
	BRDF.Albedo    = GBufferAlbedo[Pixel].rgb;
	BRDF.Roughness = RM.x;
	BRDF.Metallic  = RM.y;

	//float3 Lightmap = GBufferLightmap[Pixel].rgb;
	//float3 LightingSum = Lightmap * Albedo;
	float3 LightingSum = float3(0, 0, 0);

	// TODO: move lighting functions to a common header
    for (uint i = 0; i < GPUSceneScene[0].LightsCount; i++)
	{
		GPULight Light = GPUSceneLights[i];

		float  Shadow   = ShadowTextures[i][Pixel].r;
		float3 LightDir = Light.Direction.xyz;

		float Attenuation = 0;
		float Distance    = distance(WP, Light.Position.xyz);
		float LightRange  = Light.Range;

		switch (Light.Type)
		{
		case GPULIGHT_DIRECTIONAL:
			Attenuation = 1; // directional light doesn't attenuate
			break;
		case GPULIGHT_POINT:
			Attenuation  = clamp(1.0 - Distance * Distance / (LightRange * LightRange), 0.0, 1.0);
			Attenuation *= Attenuation;
			LightDir     = -normalize(WP - Light.Position.xyz);
			break;
		default: // TODO: support other light types
			break;
		}

		BRDF.L = LightDir;

		float3 LightValue = max(dot(BRDF.N, LightDir), 0) * Shadow * Attenuation * Light.Color.rgb;

		LightingSum += EvaluateBRDF(BRDF, LightValue);
	}

	// apply indirect
	{
		float MetalFactor = 1 - RM.y;
		LightingSum += GBufferReflections[Pixel].rgb;
		LightingSum += BRDF.Albedo * GBufferGI[Pixel].rgb * MetalFactor;
	}
	
	// TEST
    //LightingSum = GBufferLightmap[Pixel].rgb;
	
	LightingOutput[Pixel] = float4(LightingSum, 1);
}