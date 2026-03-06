#include "../Common.hlsli"

#define GPU_SCENE_NO_BINDINGS
#include "../GPUScene.hlsli"

[[vk::binding(0, 0)]] Texture2D<float4> SceneColor;
[[vk::binding(1, 0)]] Texture2D<float> SceneDepth;
[[vk::binding(2, 0)]] Texture2D<float4> SceneWorldPosition;
[[vk::binding(3, 0)]] Texture3D<float4> FroxelScattering;
[[vk::binding(4, 0)]] RWTexture2D<float4> OutputColor;
[[vk::binding(5, 0)]] RWTexture2D<float4> OutputFog;
[[vk::binding(6, 0)]] SamplerState LinearSampler;
[[vk::binding(7, 0)]] StructuredBuffer<GPUSceneStruct> GPUSceneScene;

[[vk::push_constant]]
struct _Params
{
	int2 RenderSize;
	uint FrameIndex;
	uint SampleFilter;
	float SkyTransmittance;
	float FroxelJitter;
	float2 _pad0;
} Params;

float4 SampleCatmullRomSlice(Texture3D<float4> tex, float2 uv, float normalizedSlice, float2 textureSize)
{
	float2 samplePosition = uv * textureSize;
	float2 texPos1 = floor(samplePosition - 0.5f) + 0.5f;
	float2 fractionalOffset = samplePosition - texPos1;

	float2 w0 = fractionalOffset * (-0.5f + fractionalOffset * (1.0f - 0.5f * fractionalOffset));
	float2 w1 = 1.0f + fractionalOffset * fractionalOffset * (-2.5f + 1.5f * fractionalOffset);
	float2 w2 = fractionalOffset * (0.5f + fractionalOffset * (2.0f - 1.5f * fractionalOffset));
	float2 w3 = fractionalOffset * fractionalOffset * (-0.5f + 0.5f * fractionalOffset);

	float2 w12 = w1 + w2;
	float2 offset12 = w2 / max(w12, 1e-4);

	float2 texPos0 = (texPos1 - 1.0f) / textureSize;
	float2 texPos3 = (texPos1 + 2.0f) / textureSize;
	float2 texPos12 = (texPos1 + offset12) / textureSize;

	float4 result = 0.0f;
	result += tex.SampleLevel(LinearSampler, float3(texPos0.x, texPos0.y, normalizedSlice), 0.0f) * w0.x * w0.y;
	result += tex.SampleLevel(LinearSampler, float3(texPos12.x, texPos0.y, normalizedSlice), 0.0f) * w12.x * w0.y;
	result += tex.SampleLevel(LinearSampler, float3(texPos3.x, texPos0.y, normalizedSlice), 0.0f) * w3.x * w0.y;

	result += tex.SampleLevel(LinearSampler, float3(texPos0.x, texPos12.y, normalizedSlice), 0.0f) * w0.x * w12.y;
	result += tex.SampleLevel(LinearSampler, float3(texPos12.x, texPos12.y, normalizedSlice), 0.0f) * w12.x * w12.y;
	result += tex.SampleLevel(LinearSampler, float3(texPos3.x, texPos12.y, normalizedSlice), 0.0f) * w3.x * w12.y;

	result += tex.SampleLevel(LinearSampler, float3(texPos0.x, texPos3.y, normalizedSlice), 0.0f) * w0.x * w3.y;
	result += tex.SampleLevel(LinearSampler, float3(texPos12.x, texPos3.y, normalizedSlice), 0.0f) * w12.x * w3.y;
	result += tex.SampleLevel(LinearSampler, float3(texPos3.x, texPos3.y, normalizedSlice), 0.0f) * w3.x * w3.y;

	return result;
}

float4 SampleFroxelBicubic(Texture3D<float4> tex, float3 uvw, uint3 dimensions)
{
	float depthSize = max((float)dimensions.z, 1.0f);
	float zCoord = uvw.z * depthSize - 0.5f;
	int z0 = clamp((int)floor(zCoord), 0, (int)dimensions.z - 1);
	int z1 = min(z0 + 1, (int)dimensions.z - 1);
	float zLerp = saturate(zCoord - z0);

	float z0Norm = (z0 + 0.5f) / depthSize;
	float z1Norm = (z1 + 0.5f) / depthSize;

	float4 s0 = SampleCatmullRomSlice(tex, uvw.xy, z0Norm, float2(dimensions.xy));
	float4 s1 = SampleCatmullRomSlice(tex, uvw.xy, z1Norm, float2(dimensions.xy));
	return lerp(s0, s1, zLerp);
}

float4 CatmullRomWeights(float t)
{
	float t2 = t * t;
	float t3 = t2 * t;

	float4 weights;
	weights.x = -0.5 * t + 1.0 * t2 - 0.5 * t3;
	weights.y = 1.0 - 2.5 * t2 + 1.5 * t3;
	weights.z = 0.5 * t + 2.0 * t2 - 1.5 * t3;
	weights.w = -0.5 * t2 + 0.5 * t3;
	return weights;
}

float4 SampleFroxelTricubic(Texture3D<float4> tex, float3 uvw, uint3 dimensions)
{
	float3 dim = max(float3(dimensions), 1.0);
	float3 samplePos = uvw * dim - 0.5;
	int3 base = (int3)floor(samplePos);
	float3 fracPos = samplePos - floor(samplePos);

	float4 wx = CatmullRomWeights(fracPos.x);
	float4 wy = CatmullRomWeights(fracPos.y);
	float4 wz = CatmullRomWeights(fracPos.z);

	float4 result = 0.0;

	[unroll]
	for (int z = 0; z < 4; z++)
	{
		int sampleZ = clamp(base.z + z - 1, 0, (int)dimensions.z - 1);
		float wzv = wz[z];

		[unroll]
		for (int y = 0; y < 4; y++)
		{
			int sampleY = clamp(base.y + y - 1, 0, (int)dimensions.y - 1);
			float wyv = wy[y];

			[unroll]
			for (int x = 0; x < 4; x++)
			{
				int sampleX = clamp(base.x + x - 1, 0, (int)dimensions.x - 1);
				float w = wx[x] * wyv * wzv;
				result += tex.Load(int4(sampleX, sampleY, sampleZ, 0)) * w;
			}
		}
	}

	return result;
}

[numthreads(8, 8, 1)]
void main(uint3 dtid : SV_DispatchThreadID)
{
	uint2 pixel = dtid.xy;
	if (any(pixel >= Params.RenderSize))
	{
		return;
	}

	GPUVolumetricFogSettings fog = GPUSceneScene[0].VolumetricFog;
	float4 baseColor = SceneColor[pixel];

	if (fog.Control.x == 0)
	{
		OutputColor[pixel] = baseColor;
		OutputFog[pixel] = float4(0, 0, 0, 0);
		return;
	}

	float3 cameraPos = GPUSceneScene[0].CameraCur.CameraPosition.xyz;
	float maxFogDistance = max(fog.Params0.z, 0.01);

	float surfaceDistance = maxFogDistance;
	float sceneDepth = SceneDepth[pixel];
	bool isSkyPixel = sceneDepth <= EPSILON || sceneDepth >= (1.0 - EPSILON);
	if (sceneDepth > EPSILON && sceneDepth < (1.0 - EPSILON))
	{
		float3 worldPos = SceneWorldPosition[pixel].xyz;
		surfaceDistance = min(distance(worldPos, cameraPos), maxFogDistance);
	}

	if (surfaceDistance <= EPSILON)
	{
		OutputColor[pixel] = baseColor;
		OutputFog[pixel] = float4(0, 0, 0, 0);
		return;
	}

	uint steps = max((uint)fog.Control.w, 1u);
	float stepSize = surfaceDistance / steps;

	float2 uv = (pixel + 0.5) / float2(Params.RenderSize);
	uint froxelWidth, froxelHeight, froxelDepth;
	FroxelScattering.GetDimensions(froxelWidth, froxelHeight, froxelDepth);
	float2 froxelTexel = 1.0 / max(float2(froxelWidth, froxelHeight), 1.0);

	uint rngState = Random::Hash(Random::Hash(pixel.x) + Random::Hash(pixel.y) + Params.FrameIndex);
	float jitterStrength = max(Params.FroxelJitter, 0.0);

	float transmittance = 1.0;
	float3 inScattering = float3(0, 0, 0);

	for (uint s = 0; s < steps; s++)
	{
		float stepJitter = (Random::StepAndOutputRNGFloat(rngState) - 0.5) * jitterStrength;
		float distanceToSample = ((s + 0.5) + stepJitter) * stepSize;
		distanceToSample = clamp(distanceToSample, 0.0, surfaceDistance);

		float sliceT = sqrt(saturate(distanceToSample / maxFogDistance));

		float4 sigma = 0;
		if (Params.SampleFilter == 3u)
		{
			sigma = SampleFroxelTricubic(FroxelScattering, float3(uv, sliceT), uint3(froxelWidth, froxelHeight, froxelDepth));
		}
		else if (Params.SampleFilter == 2u)
		{
			sigma = SampleFroxelBicubic(FroxelScattering, float3(uv, sliceT), uint3(froxelWidth, froxelHeight, froxelDepth));
		}
		else if (Params.SampleFilter == 1u)
		{
			float2 stochasticOffset = (Random::UniformDistrubition2d(rngState) - 0.5) * froxelTexel * jitterStrength;
			float2 uv0 = saturate(uv + stochasticOffset);
			float2 uv1 = saturate(uv - stochasticOffset);
			float4 sigma0 = FroxelScattering.SampleLevel(LinearSampler, float3(uv0, sliceT), 0);
			float4 sigma1 = FroxelScattering.SampleLevel(LinearSampler, float3(uv1, sliceT), 0);
			sigma = 0.5 * (sigma0 + sigma1);
		}
		else
		{
			sigma = FroxelScattering.SampleLevel(LinearSampler, float3(uv, sliceT), 0);
		}

		sigma = max(sigma, 0.0);
		float sigmaT = max(sigma.a, 0.0);
		float attenuation = exp(-sigmaT * stepSize);

		if (sigmaT > 1e-4)
		{
			inScattering += transmittance * sigma.rgb * (1.0 - attenuation) / sigmaT;
		}
		else
		{
			inScattering += transmittance * sigma.rgb * stepSize;
		}

		transmittance *= attenuation;
		if (transmittance < 0.001)
		{
			break;
		}
	}

	float fogAlpha = saturate(1.0 - transmittance);
	float4 fogVolume = saturate(float4(inScattering, fogAlpha));

	float transmittanceForColor = saturate(1.0 - fogVolume.a);
	if (isSkyPixel)
	{
		transmittanceForColor = lerp(1.0, transmittanceForColor, Params.SkyTransmittance);
	}

	float3 finalColor = baseColor.rgb * transmittanceForColor + fogVolume.rgb;

	OutputColor[pixel] = float4(finalColor, baseColor.a);
	OutputFog[pixel] = fogVolume;
}
