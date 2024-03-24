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
	layout(binding = 1, set = 2, rgba16f) uniform image2D ResultReflections;
	layout(binding = 2, set = 2) uniform sampler2D GBufferAlbedo;
	layout(binding = 3, set = 2) uniform sampler2D GBufferNormals;
	layout(binding = 4, set = 2) uniform sampler2D GBufferWorldPosition;
	layout(binding = 5, set = 2) uniform sampler2D GBufferRoughnessMetallic;
	layout(binding = 6, set = 2) uniform sampler2D GBufferDepth;
	
	#include "../GPUScene.glsl" // TODO:
	#include "../BRDF.glsl"
	#include "../Common.glsl"

	#define EPSILON 0.0001

	layout(push_constant) uniform params
	{
		vec4 CameraPosition;
		uint Random;
	} Params;

	void main()
	{
		const uvec2 pixel = gl_LaunchIDEXT.xy;
		const vec2 uv = vec2(gl_LaunchIDEXT.xy) / vec2(gl_LaunchSizeEXT.xy - 1);

		float depth = texture(GBufferDepth, uv).x;
		// do not trace from sky
		if (abs(depth) < EPSILON || abs(depth - 1) < EPSILON)
		{
			imageStore(ResultReflections, ivec2(gl_LaunchIDEXT), vec4(0));
			return;
		}

		uint RngState = hash(hash(pixel.x) + hash(pixel.y) + (Params.Random)); // Initial seed

		vec3 Origin = texture(GBufferWorldPosition, uv).xyz;
		vec3 Normal = texture(GBufferNormals, uv).xyz;
		vec3 Direction = normalize(Origin - Params.CameraPosition.xyz);
		vec3 Albedo = texture(GBufferAlbedo, uv).rgb;
		vec2 RM = texture(GBufferRoughnessMetallic, uv).xy;

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
		vec3 RayAttenuation = EvaluateBRDF(BRDF, vec3(1)) * NdotL / Sample.Pdf;

		float MaxDistance = 5000; // TODO: make it a parameter

		traceRayEXT(acc, gl_RayFlagsOpaqueEXT,
			0xFF, 0, 0, 0, Origin, 0.01, Direction, MaxDistance, 0);

		vec3 Radiance = vec3(0);

		// evaluate lighting at the hit point
		if (Payload.colorAndDist.w > 0)
		{
			BRDF.Albedo = Payload.colorAndDist.rgb;
			BRDF.N = Payload.normalAndObjId.xyz;
			BRDF.V = -Direction;
			BRDF.Roughness = Payload.RoughnessMetallic.x;
			BRDF.Metallic = Payload.RoughnessMetallic.y;

			vec3 HitPoint = Payload.colorAndDist.w * Direction + Origin;
			Origin = HitPoint + BRDF.N * 0.001;

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

		imageStore(ResultReflections, ivec2(gl_LaunchIDEXT), vec4(FinalResult, 1));
		// imageStore(ResultReflections, ivec2(gl_LaunchIDEXT), vec4(Direction, 1));
	}
#endif

#ifdef MISS_SHADER
	#include "../PathTraceMissing.glsl"
#endif

#ifdef CLOSEST_HIT_SHADER
	#include "../PathTraceClosestHit.glsl"
#endif