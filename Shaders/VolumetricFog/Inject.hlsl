struct Payload
{
	float HitT;
};

#ifdef RAYGEN_SHADER
#include "../GPUScene.hlsli"
#include "../Common.hlsli"

#define SET 2

[[vk::binding(0, SET)]] RaytracingAccelerationStructure                     AccelerationStructure;
[[vk::binding(1, SET)]] [[vk::image_format("rgba16f")]] RWTexture3D<float4> OutputFroxels;
[[vk::binding(2, SET)]] Texture3D<float4>                                   HistoryFroxels;
[[vk::binding(3, SET)]] SamplerState                                        LinearSampler;

[[vk::push_constant]]
struct _Params
{
	uint FrameIndex;
	uint MaxLights;
	uint ShadowSamples;
	uint Flags;
	float ShadowJitter;
	float FroxelJitter;
	float TemporalBlendFactor;
	float HistoryClip;
} Params;

float PhaseHG(float cosTheta, float g)
{
	float g2 = g * g;
	float denom = pow(max(1.0 + g2 - 2.0 * g * cosTheta, 1e-4), 1.5);
	return (1.0 - g2) / (4.0 * PI * denom);
}

float Hash3D(float3 p)
{
	return frac(sin(dot(p, float3(12.9898, 78.233, 37.719))) * 43758.5453);
}

float ComputeRangeAttenuation(float distanceToLight, float range)
{
	float r = max(range, 0.001);
	float attenuation = saturate(1.0 - (distanceToLight * distanceToLight) / (r * r));
	return attenuation * attenuation;
}

float TraceShadow(float3 samplePos, float3 toLightDir, float maxDistance, GPULight light, float shadowJitter, inout uint rngState)
{
	float3 rayDirection = toLightDir;

	float coneRadius = 0.0;
	if (light.SourceRadius > 0.0)
	{
		coneRadius = (light.Type == GPULIGHT_DIRECTIONAL) ? light.SourceRadius : light.SourceRadius / max(maxDistance, 0.001);
	}

	if (shadowJitter > 0.0)
	{
		float jitterCone = (light.Type == GPULIGHT_DIRECTIONAL) ? (shadowJitter * 0.005) : (shadowJitter * 0.15 / max(maxDistance, 0.5));
		coneRadius += jitterCone;
	}

	if (coneRadius > 0.0)
	{
		float2 rnd = Random::UniformDistrubition2d(rngState);
		rayDirection = normalize(Random::RandomDirectionCone(toLightDir, min(coneRadius, 0.5), rnd));
	}

	RayDesc ray;
	ray.Origin = samplePos + rayDirection * 0.05;
	ray.TMin = 0.0;
	ray.Direction = rayDirection;
	ray.TMax = max(maxDistance - 0.05, 0.01);

	Payload payload;
	payload.HitT = -1.0;

	TraceRay(AccelerationStructure, RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH,
		~0, 0, 0, 0, ray, payload);

	return payload.HitT > 0.0 ? 0.0 : 1.0;
}

[shader("raygeneration")]
void RayGen()
{
	uint3 froxel = DispatchRaysIndex().xyz;
	uint3 froxelDimensions = DispatchRaysDimensions().xyz;

	if (any(froxel >= froxelDimensions))
	{
		return;
	}

	GPUVolumetricFogSettings fog = GPUScene::GPUSceneScene[0].VolumetricFog;
	if (fog.Control.x == 0)
	{
		OutputFroxels[froxel] = float4(0, 0, 0, 0);
		return;
	}

	uint rngState = Random::Hash(Random::Hash(froxel.x) + Random::Hash(froxel.y) + Random::Hash(froxel.z) + Params.FrameIndex);
	float froxelJitter = max(Params.FroxelJitter, 0.0);

	float2 uv = (froxel.xy + 0.5) / float2(froxelDimensions.xy);
	float2 uvJitter = (Random::UniformDistrubition2d(rngState) - 0.5) * froxelJitter / max(float2(froxelDimensions.xy), 1.0);
	uv = saturate(uv + uvJitter);

	float4 ndc = float4(uv * 2.0 - 1.0, 1.0, 1.0);
	ndc.y *= -1.0;

	float4 worldFar = mul(ndc, GPUScene::GPUSceneScene[0].CameraCur.InverseViewProjectionMatrix);
	worldFar /= worldFar.w;

	float3 cameraPos = GPUScene::GPUSceneScene[0].CameraCur.CameraPosition.xyz;
	float3 rayDir = normalize(worldFar.xyz - cameraPos);

	float maxFogDistance = max(fog.Params0.z, 0.01);
	float sliceJitter = (Random::StepAndOutputRNGFloat(rngState) - 0.5) * froxelJitter;
	float sliceT = ((froxel.z + 0.5) + sliceJitter) / max((float)froxelDimensions.z, 1.0);
	sliceT = saturate(sliceT);
	float distanceToSample = sliceT * sliceT * maxFogDistance;
	float3 samplePos = cameraPos + rayDir * distanceToSample;

	float density = fog.AlbedoDensity.w;

	float heightExponent = -(samplePos.y - fog.Params0.y) * fog.Params0.x;
	heightExponent = clamp(heightExponent, -20.0, 20.0);
	density *= exp(heightExponent);

	float noise = Hash3D(samplePos * fog.Params1.x);
	float noiseFactor = lerp(1.0 - fog.Params1.y, 1.0 + fog.Params1.y, noise);
	density = max(density * noiseFactor, 0.0);

	float3 lighting = float3(0, 0, 0);

	uint lightsCount = min(GPUScene::GPUSceneScene[0].LightsCount, Params.MaxLights);
	uint shadowSamples = max(Params.ShadowSamples, 1u);
	for (uint i = 0; i < lightsCount; i++)
	{
		GPULight light = GPUScene::GPUSceneLights[i];
		if (light.VolumetricIntensity <= 0.0)
		{
			continue;
		}

		float3 toLightDir = normalize(light.Direction.xyz);
		float maxDistance = 5000.0;
		float attenuation = 1.0;

		if (light.Type == GPULIGHT_POINT || light.Type == GPULIGHT_SPOT || light.Type == GPULIGHT_RECTANGLE || light.Type == GPULIGHT_DISC || light.Type == GPULIGHT_LINE)
		{
			float3 toLight = light.Position.xyz - samplePos;
			float distanceToLight = length(toLight);
			if (distanceToLight >= light.Range || distanceToLight <= EPSILON)
			{
				continue;
			}

			toLightDir = toLight / distanceToLight;
			maxDistance = distanceToLight;
			attenuation = ComputeRangeAttenuation(distanceToLight, light.Range);

			if (light.Type == GPULIGHT_SPOT)
			{
				float angle = saturate(dot(toLightDir, normalize(light.Direction.xyz)));
				attenuation *= smoothstep(light.SizeOrSpotAngles.y, light.SizeOrSpotAngles.x, angle);
			}

			if (attenuation <= 0.0)
			{
				continue;
			}
		}

		float shadow = 1.0;
		if ((light.Flags & GPULIGHT_FLAG_SHADOW) != 0)
		{
			shadow = 0.0;
			[loop]
			for (uint sample = 0; sample < shadowSamples; sample++)
			{
				shadow += TraceShadow(samplePos, toLightDir, maxDistance, light, Params.ShadowJitter, rngState);
			}
			shadow /= shadowSamples;
		}

		float shadowTerm = lerp(1.0, shadow, fog.Params1.z);
		float3 viewDir = normalize(cameraPos - samplePos);
		float mu = dot(-toLightDir, viewDir);
		float phase = PhaseHG(mu, fog.Params0.w);

		lighting += light.Color.rgb * light.VolumetricIntensity * attenuation * shadowTerm * phase;
	}

	float3 sigmaS = lighting * density * fog.AlbedoDensity.rgb;
	float sigmaT = density;
	float4 currentFroxel = float4(max(sigmaS, 0.0), max(sigmaT, 0.0));

	const bool hasHistory = (Params.Flags & 1u) != 0u;
	if (hasHistory)
	{
		float4 prevClip = mul(float4(samplePos, 1.0), GPUScene::GPUSceneScene[0].CameraPrev.ViewProjectionMatrix);
		if (abs(prevClip.w) > EPSILON)
		{
			float2 prevNdc = prevClip.xy / prevClip.w;
			float2 prevUv = prevNdc * float2(0.5, -0.5) + 0.5;
			if (all(prevUv >= 0.0) && all(prevUv <= 1.0))
			{
				float3 prevCameraPos = GPUScene::GPUSceneScene[0].CameraPrev.CameraPosition.xyz;
				float prevDistance = distance(samplePos, prevCameraPos);
				float prevSliceT = sqrt(saturate(prevDistance / maxFogDistance));

				float4 historyFroxel = HistoryFroxels.SampleLevel(LinearSampler, float3(prevUv, prevSliceT), 0.0);
				historyFroxel = max(historyFroxel, 0.0);

				float historyClip = max(Params.HistoryClip, 0.001);
				float4 historyMin = max(currentFroxel - historyClip, 0.0);
				float4 historyMax = currentFroxel + historyClip;
				historyFroxel = clamp(historyFroxel, historyMin, historyMax);

				float historyWeight = saturate(Params.TemporalBlendFactor);
				currentFroxel = lerp(currentFroxel, historyFroxel, historyWeight);
			}
		}
	}

	OutputFroxels[froxel] = currentFroxel;
}
#endif

[shader("miss")]
void Miss(inout Payload payload)
{
	payload.HitT = -1.0;
}

[shader("closesthit")]
void ClosestHit(inout Payload payload, BuiltInTriangleIntersectionAttributes attrib)
{
	payload.HitT = RayTCurrent();
}
