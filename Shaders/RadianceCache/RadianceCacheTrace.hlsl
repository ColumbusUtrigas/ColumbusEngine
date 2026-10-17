struct RayPayload
{
	float3 Colour;
	float3 Emissive;
	float  HitDistance;
	float3 Normal;
	uint   ObjectId;
	float2 RoughnessMetallic;
	float3 GeometricNormal;
};

#define CACHE_DIFFUSE_MATERIAL_HIT
#define LIGHTWEIGHT_SHADOW_HITS 1
#define PAYLOAD_HAS_GEOMETRIC_NORMAL
#define SHARC_UPDATE 1
#define SHARC_QUERY 0
#include "../RayTracingShadowVisibility.hlsli"
#include "../GPUScene.hlsli"
#include "../CommonRayTracing.hlsli"
#include "../SkyCommon.hlsli"
#include "RadianceCache.hlsli"

[[vk::binding(0, 2)]] RaytracingAccelerationStructure AccelerationStructure;
[[vk::binding(1, 2)]] Texture2D<float> GBufferDepth;
[[vk::binding(2, 2)]] Texture2D<float2> GBufferNormal;
[[vk::binding(3, 2)]] Texture2D<float3> GBufferAlbedo;
[[vk::binding(4, 2)]] Texture2D<float4> GBufferRoughnessMetallic;

// Store unit-albedo diffuse lighting. Material colour is applied only to transport
// through previously visited surfaces, never baked into the local cache entry.
float3 TraceDirectDiffuseLighting(float3 Position, float3 Normal, inout uint RngState)
{
	float3 Lighting = 0;

	for (uint i = 0; i < GPUScene::GetLightsCount(); ++i)
	{
		const float2 Xi = Random::UniformDistrubition2d(RngState);
		const RayTracedLightSample Light = SampleRayTracedLight(GPUScene::GPUSceneLights[i], Position, Xi);
		const float Cosine = dot(Normal, Light.Direction);

		if (!Light.Valid || Cosine <= 0 || !any(Light.Radiance > 0))
		{
			continue;
		}

		const float Visibility = TraceShadowRay(AccelerationStructure, Position, Light.Direction, Light.Distance);
		Lighting += Light.Radiance * (Cosine * ONE_OVER_PI * Visibility);
	}

	return SanitizeRadiance(Lighting);
}

#ifdef RAYGEN_SHADER
[shader("raygeneration")]
void RayGen()
{
	const uint2 GridSize = RadianceCacheConfig.Update.xy;
	const uint2 Cell     = DispatchRaysIndex().xy;
	const uint2 Size     = uint2(GPUScene::GPUSceneScene[0].RenderSize);

	// Integer boundaries partition the full GBuffer, including partial edge cells.
	const uint2 Begin  = Cell * Size / GridSize;
	const uint2 End    = (Cell + 1) * Size / GridSize;
	const uint2 Extent = End - Begin;

	// Visit every pixel in each cell over time. Different cell phases avoid moving
	// the entire sampling pattern in lockstep, and require no persistent sampling data.
	const uint Frame  = RadianceCacheConfig.Config.y;
	const uint Phase  = Random::Hash(Cell.x + Cell.y * GridSize.x);
	const uint Sample = (Frame + Phase) % (Extent.x * Extent.y);
	const uint2 Pixel = Begin + uint2(Sample % Extent.x, Sample / Extent.x);

	if (IsSkyDepth(GBufferDepth[Pixel]))
	{
		return;
	}

	uint RngState = Random::Hash(Pixel.x + Pixel.y * Size.x + Random::Hash(Frame));
	float3 Position = ReconstructWorldPositionFromDepth(Pixel, GBufferDepth[Pixel], Size, GPUScene::GPUSceneScene[0].CameraCur.InverseViewProjectionMatrix);
	float3 Normal = NormalDecode(GBufferNormal[Pixel]);
	BRDFData BRDF;
	BRDF.N         = Normal;
	BRDF.V         = normalize(RadianceCacheConfig.CameraPositionScale.xyz - Position);
	BRDF.Albedo    = GBufferAlbedo[Pixel];
	BRDF.Roughness = GBufferRoughnessMetallic[Pixel].x;
	BRDF.Metallic  = GBufferRoughnessMetallic[Pixel].y;

	float3 Direction;
	const bool ReflectionPath = RadianceCacheConfig.Flags.y != 0 && (RadianceCacheConfig.Flags.z == 0 || (DispatchRaysIndex().x + DispatchRaysIndex().y + Frame) % 2 == 0);

	if (ReflectionPath)
	{
		Direction = SampleBRDF_GGX(BRDF, Random::UniformDistrubition2d(RngState)).Dir;
	}
	else
	{
		Direction = SampleBRDF_Lambert(Normal, Random::UniformDistrubition2d(RngState)).Dir;
	}

	SharcParameters Cache = GetRadianceCacheParameters();
	SharcState State;
	SharcInit(State);

	for (uint i = 0; i < uint(RadianceCacheConfig.Update.w); ++i)
	{
		RayDesc Ray;
		Ray.Origin    = Position + Normal * 0.01f;
		Ray.TMin      = 0.001f;
		Ray.Direction = Direction;
		Ray.TMax      = 5000;
		RayPayload Payload;
		TraceRay(AccelerationStructure, RAY_FLAG_FORCE_OPAQUE, 0xff, 0, 0, 0, Ray, Payload);

		if (Payload.HitDistance < 0)
		{
			SharcUpdateMiss(Cache, State, SanitizeRadiance(Payload.Colour));
			break;
		}

		Position = Ray.Origin + Direction * Payload.HitDistance;
		Normal   = Payload.GeometricNormal;

		// Emission belongs to this material, so propagate it to earlier vertices only.
		SharcUpdateMiss(Cache, State, SanitizeRadiance(Payload.Emissive));
		SharcSetThroughput(State, saturate(Payload.Colour));

		const float3 Lighting = TraceDirectDiffuseLighting(Position + Normal * 0.01f, Normal, RngState);
		SharcHitData Hit = MakeRadianceCacheHit(Position, Normal);

		if (!SharcUpdateHit(Cache, State, Hit, Lighting, Random::StepAndOutputRNGFloat(RngState)))
		{
			break;
		}

		// Cosine-sampled Lambert transport has unit throughput for the local white
		// lighting entry. The next hit applies its actual albedo to existing paths.
		Direction = SampleBRDF_Lambert(Normal, Random::UniformDistrubition2d(RngState)).Dir;
	}
}
#endif

[shader("miss")]
void Miss(inout RayPayload Payload)
{
	Payload.HitDistance = -1;

	if (IsOpaqueShadowRay())
	{
		return;
	}

	const float3 Sun = normalize(GPUScene::GPUSceneScene[0].SunDirection.xyz);
	Payload.Colour = Sky::Atmosphere(WorldRayOrigin(), WorldRayDirection(), Sun, GPUScene::GPUSceneScene[0].Sky);
}

#include "../RayTracingClosestHit.hlsli"
