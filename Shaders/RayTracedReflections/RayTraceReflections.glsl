#version 460 core
#extension GL_EXT_ray_tracing : enable
#extension GL_GOOGLE_include_directive : require

struct RayPayload {
	vec4 colorAndDist;
	vec4 normalAndObjId;
	vec2 RoughnessMetallic;
};

#ifdef RAYGEN_SHADER
	layout(location = 0) rayPayloadEXT RayPayload Payload;
	layout(location = 1) rayPayloadEXT RayPayload shadowPayload; // TODO:

	layout(binding = 0, set = 2) uniform accelerationStructureEXT acc; // TODO:
	layout(binding = 1, set = 2, rgba16f) uniform image2D ResultRadiance;
	layout(binding = 2, set = 2, rgba16f) uniform image2D ResultDirectionDistance;
	layout(binding = 3, set = 2) uniform sampler2D GBufferAlbedo;
	layout(binding = 4, set = 2) uniform sampler2D GBufferNormals;
	layout(binding = 5, set = 2) uniform sampler2D GBufferWorldPosition;
	layout(binding = 6, set = 2) uniform sampler2D GBufferRoughnessMetallic;
	layout(binding = 7, set = 2) uniform sampler2D GBufferDepth;

	#define RADIANCE_CACHE_BINDING 8
	#define RADIANCE_CACHE_SET 2
	#include "../RadianceCache/RadianceCache.glsl"
	
	#include "../GPUScene.glsl" // TODO:
	#include "../BRDF.glsl"
	#include "../Common.glsl"

	layout(push_constant) uniform params
	{
		vec4  CameraPosition;
		float MaxRoughness;
		uint  Random;
		uint  UseRadianceCache;
	} Params;

	void main()
	{
		const uvec2 pixel = gl_LaunchIDEXT.xy;
		const vec2 uv = vec2(gl_LaunchIDEXT.xy) / vec2(gl_LaunchSizeEXT.xy - 1);

		float depth = texture(GBufferDepth, uv).x;
		// do not trace from sky
		if (abs(depth) < EPSILON || abs(depth - 1) < EPSILON)
		{
			imageStore(ResultRadiance, ivec2(gl_LaunchIDEXT), vec4(0));
			imageStore(ResultDirectionDistance, ivec2(gl_LaunchIDEXT), vec4(-1));
			return;
		}

		uint RngState = hash(hash(pixel.x) + hash(pixel.y) + (Params.Random)); // Initial seed

		vec3 Origin = texture(GBufferWorldPosition, uv).xyz;
		vec3 Normal = texture(GBufferNormals, uv).xyz;
		vec3 Direction = normalize(Origin - Params.CameraPosition.xyz);
		vec3 Albedo = texture(GBufferAlbedo, uv).rgb;
		vec2 RM = texture(GBufferRoughnessMetallic, uv).xy;

		// roughness cut
		if (RM.x > Params.MaxRoughness)
		{
			imageStore(ResultRadiance, ivec2(gl_LaunchIDEXT), vec4(0));
			imageStore(ResultDirectionDistance, ivec2(gl_LaunchIDEXT), vec4(-1));
			return;
		}

		BRDFData BRDF;
		BRDF.N = Normal;
		BRDF.V = -Direction;
		BRDF.Albedo = Albedo;
		BRDF.Roughness = RM.r;
		BRDF.Metallic = RM.g;

		BRDFSample Sample = SampleBRDF_GGX(BRDF, UniformDistrubition2d(RngState));
		// Direction = reflect(Direction, Normal);
		Direction = Sample.Dir;
		BRDF.L = Direction;

		float NdotL = max(0, dot(BRDF.N, BRDF.L));
		//vec3 RayAttenuation = EvaluateBRDF(BRDF, vec3(1)) * NdotL / Sample.Pdf;
		vec3 RayAttenuation = EvaluateBRDF(BRDF, vec3(1)) / max(Sample.Pdf, 0.001);

		float MaxDistance = 5000; // TODO: make it a parameter

		traceRayEXT(acc, gl_RayFlagsOpaqueEXT,
			0xFF, 0, 0, 0, Origin, 0.01, Direction, MaxDistance, 0);

		float RayDistance = Payload.colorAndDist.w;
		vec3 Radiance = vec3(0);

		// evaluate lighting at the hit point
		if (RayDistance > 0)
		{
			BRDF.Albedo = Payload.colorAndDist.rgb;
			BRDF.N = Payload.normalAndObjId.xyz;
			BRDF.V = -Direction;
			BRDF.Roughness = Payload.RoughnessMetallic.x;
			BRDF.Metallic = Payload.RoughnessMetallic.y;

			vec3 HitPoint = RayDistance * Direction + Origin;
			Origin = HitPoint + BRDF.N * 0.001;

			if (Params.UseRadianceCache > 0)
			{
				Radiance += SampleRadianceCache(Params.CameraPosition.xyz, HitPoint);
			}

			// TODO: sample lighting from GI
			// TODO: unify light calculation between different RT passes into a function
			for (uint l = 0; l < GPUScene_GetLightsCount(); l++)
			{
				vec3 LightSample = vec3(0);

				// TODO: get it from sample functions
				BRDF.L = vec3(0); // light direction

				vec2 Xi = UniformDistrubition2d(RngState);
				// TODO: light PDF, as every time we do a random decision, we need to weight it by it's probability

				GPULight Light = GPUSceneLights.Lights[l];
				switch (Light.Type)
				{
				case GPULIGHT_DIRECTIONAL:
					LightSample = SampleDirectionalLight(Light, Origin, BRDF.N, Xi);
					BRDF.L = Light.Direction.xyz;
					break;
				case GPULIGHT_POINT:
					LightSample = SamplePointLight(Light, Origin, BRDF.N, Xi);
					// TODO: account for sphere light
					BRDF.L = normalize(Light.Position.xyz - Origin);
					break;
				}

				Radiance += EvaluateBRDF(BRDF, vec3(1)) * LightSample;
			}
		}
		else
		{
			Radiance += Payload.colorAndDist.rgb;
		}

		vec3 FinalResult = Radiance * RayAttenuation;

		imageStore(ResultRadiance, ivec2(gl_LaunchIDEXT), vec4(FinalResult, 1));
		imageStore(ResultDirectionDistance, ivec2(gl_LaunchIDEXT), vec4(Sample.Dir, RayDistance));
	}
#endif

#ifdef MISS_SHADER
	#include "../PathTraceMissing.glsl"
#endif

#ifdef CLOSEST_HIT_SHADER
	#include "../PathTraceClosestHit.glsl"
#endif