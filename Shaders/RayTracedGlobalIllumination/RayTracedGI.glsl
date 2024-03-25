#version 460 core
#extension GL_EXT_ray_tracing : enable
#extension GL_GOOGLE_include_directive : require

struct RayPayload
{
	vec4 colorAndDist;
	vec4 normalAndObjId;
	vec2 RoughnessMetallic;
};

#ifdef RAYGEN_SHADER
	layout(location = 0) rayPayloadEXT RayPayload Payload;
	layout(location = 1) rayPayloadEXT RayPayload shadowPayload; // TODO:

	layout(binding = 0, set = 2) uniform accelerationStructureEXT acc; // TODO:
	layout(binding = 1, set = 2, rgba16f) uniform image2D Result;
	layout(binding = 2, set = 2) uniform sampler2D GBufferAlbedo;
	layout(binding = 3, set = 2) uniform sampler2D GBufferNormals;
	layout(binding = 4, set = 2) uniform sampler2D GBufferWorldPosition;
	layout(binding = 5, set = 2) uniform sampler2D GBufferRoughnessMetallic;
	layout(binding = 6, set = 2) uniform sampler2D GBufferDepth;
	
	#include "../GPUScene.glsl" // TODO:
	#include "../BRDF.glsl"
	#include "../Common.glsl"

	layout(push_constant) uniform params
	{
		uint Random;
	} Params;

	void main()
	{
		const ivec2 Pixel = ivec2(gl_LaunchIDEXT.xy);

		float Depth = texelFetch(GBufferDepth, Pixel, 0).x;

		// do not trace from sky
		if (abs(Depth) < EPSILON || abs(Depth - 1) < EPSILON)
		{
			imageStore(Result, ivec2(Pixel), vec4(0));
			return;
		}

		uint RngState = hash(hash(Pixel.x) + hash(Pixel.y) + (Params.Random)); // Initial seed

		// sample GBuffer
		vec3 Origin = texelFetch(GBufferWorldPosition, Pixel, 0).xyz;
		vec3 Normal = texelFetch(GBufferNormals, Pixel, 0).xyz;
		vec3 Albedo = texelFetch(GBufferAlbedo, Pixel, 0).rgb;
		vec2 RM = texelFetch(GBufferRoughnessMetallic, Pixel, 0).xy;

		BRDFSample Sample = SampleBRDF_Lambert(Normal, UniformDistrubition2d(RngState));

		float MaxDistance = 5000; // TODO: make it a parameter

		traceRayEXT(acc, gl_RayFlagsOpaqueEXT, 0xFF, 0, 0, 0, Origin, 0.01, Sample.Dir, MaxDistance, 0);

		vec3 Radiance = vec3(0);

		// evaluate lighting at the hit point
		// TODO: make it a function
		if (Payload.colorAndDist.w > 0)
		{
			BRDFData BRDF;
			BRDF.Albedo = Payload.colorAndDist.rgb;
			BRDF.N = Payload.normalAndObjId.xyz;
			BRDF.V = -Sample.Dir;
			BRDF.Roughness = Payload.RoughnessMetallic.x;
			BRDF.Metallic = Payload.RoughnessMetallic.y;

			vec3 HitPoint = Payload.colorAndDist.w * Sample.Dir + Origin;
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

		float RayDist = Payload.colorAndDist.w;
		float NdotL = max(0, dot(Normal, Sample.Dir));
		vec3 Irradiance = Radiance * LambertDiffuseBRDF(Albedo) * NdotL / Sample.Pdf;

		imageStore(Result, ivec2(Pixel), vec4(Irradiance, RayDist));
	}
#endif

#ifdef MISS_SHADER
	#include "../PathTraceMissing.glsl"
#endif

#ifdef CLOSEST_HIT_SHADER
	#include "../PathTraceClosestHit.glsl"
#endif