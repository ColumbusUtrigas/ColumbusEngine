#version 460 core
#extension GL_GOOGLE_include_directive : require

#include "BRDF.glsl"

#ifdef COMPUTE_SHADER
	layout(binding = 0, set = 0) uniform sampler2D GBufferAlbedo;
	layout(binding = 1, set = 0) uniform sampler2D GBufferNormal;
	layout(binding = 2, set = 0, rgba16f) uniform image2D LightingOutput;
	layout(binding = 3, set = 0) uniform sampler2D GBufferWorldPosition;
	layout(binding = 4, set = 0) uniform sampler2D GBufferRoughnessMetallic;
	layout(binding = 5, set = 0) uniform sampler2D GBufferLightmap;
	layout(binding = 6, set = 0, rgba16f) uniform image2D GBufferReflections;

	// TODO: use GPUScene common definitions
	struct GPULight
	{
		vec4 Position;
		vec4 Direction;
		vec4 Color;
		uint Type;
		float Range;
	};

	#define GPULIGHT_DIRECTIONAL 0
	#define GPULIGHT_POINT 1
	#define GPULIGHT_SPOT 2
	#define GPULIGHT_RECTANGLE 3
	#define GPULIGHT_SPHERE 4

	layout(binding = 7, set = 0) readonly buffer LightsBuffer {
		GPULight Lights[];
	} GPUSceneLights;

	layout(binding = 0, set = 1, r8) uniform image2D ShadowTextures[1000];

	// TODO: create a global GPUScene/View cbuffer
	layout(push_constant) uniform Params
	{
		vec4 CameraPosition;
		uint LightsCount;
	} Parameters;

	layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

	void main()
	{
		vec2 UV = (vec2(gl_GlobalInvocationID.xy) + 0.5) / vec2(gl_NumWorkGroups.xy) / vec2(gl_WorkGroupSize.xy);
		if (UV.x >= 1 && UV.y >= 1)
			return;

		vec3 Albedo = texture(GBufferAlbedo, UV).rgb;
		vec3 N = texture(GBufferNormal, UV).rgb;
		vec3 WorldPosition = texture(GBufferWorldPosition, UV).rgb;
		vec3 Lightmap = texture(GBufferLightmap, UV).rgb;
		vec3 LightingSum = Lightmap * Albedo;
		vec2 RM = texture(GBufferRoughnessMetallic, UV).rg;

		BRDFData BRDF;
		BRDF.N = N;
		BRDF.V = -normalize(WorldPosition - Parameters.CameraPosition.xyz);
		BRDF.Albedo = Albedo;
		BRDF.Roughness = RM.x;
		BRDF.Metallic = RM.y;

		// TODO: move lighting functions to a common header
		for (uint i = 0; i < Parameters.LightsCount; i++)
		{
			GPULight Light = GPUSceneLights.Lights[i];

			float Shadow = imageLoad(ShadowTextures[i], ivec2(gl_GlobalInvocationID.xy)).r;
			vec3 LightDir = Light.Direction.xyz;

			float Attenuation = 0;
			float Distance = distance(WorldPosition, Light.Position.xyz);
			float LightRange = Light.Range;

			switch (Light.Type)
			{
			case GPULIGHT_DIRECTIONAL:
				Attenuation = 1; // directional light doesn't attenuate
				break;
			case GPULIGHT_POINT:
				Attenuation = clamp(1.0 - Distance * Distance / (LightRange * LightRange), 0.0, 1.0);
				Attenuation *= Attenuation;
				LightDir = -normalize(WorldPosition - Light.Position.xyz);
				break;
			default: // TODO: support other light types
				break;
			}

			BRDF.L = LightDir;

			// vec3 LightValue = Albedo * Shadow * Attenuation * Light.Color.rgb;
			vec3 LightValue = vec3(max(dot(N, LightDir), 0)) * Albedo * Shadow * Attenuation * Light.Color.rgb;

			// LightingSum += LightValue;
			LightingSum += EvaluateBRDF(BRDF, LightValue);
		}

		LightingSum += imageLoad(GBufferReflections, ivec2(gl_GlobalInvocationID.xy)).rgb;

		imageStore(LightingOutput, ivec2(gl_GlobalInvocationID.xy), vec4(LightingSum, 1));
	}
#endif
