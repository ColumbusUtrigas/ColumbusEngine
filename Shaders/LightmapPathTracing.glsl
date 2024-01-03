#version 460 core
#extension GL_EXT_ray_tracing : enable
#extension GL_GOOGLE_include_directive : require

struct RayPayload {
	vec4 colorAndDist;
	vec4 normalAndObjId;
};

#ifdef RAYGEN_SHADER
	layout(location = 0) rayPayloadEXT RayPayload payload;
	layout(location = 1) rayPayloadEXT RayPayload shadowPayload;

	layout(binding = 0, set = 2) uniform accelerationStructureEXT acc; // TODO
	layout(binding = 1, set = 2) uniform sampler2D PositionsImage;
	layout(binding = 2, set = 2) uniform sampler2D NormalsImage;
	layout(binding = 3, set = 2) uniform sampler2D ValidityImage;
	layout(binding = 4, set = 2, rgba32f) uniform image2D LightmapOutput;

	#include "GPUScene.glsl"
	#include "CommonRayTracing.glsl"

	layout(push_constant) uniform params
	{
		int Random;
		int Bounces;
		int AccumulatedSamples;
		int RequestedSamples;
		int SamplesPerFrame;
	} Parameters;

	void main()
	{
		const ivec2 pixel = ivec2(gl_LaunchIDEXT.xy);
		const vec2 uv = vec2(gl_LaunchIDEXT.xy) / vec2(gl_LaunchSizeEXT.xy - 1);

		uint rngState = gl_LaunchIDEXT.x * pixel.y + pixel.x * (Parameters.Random + 1);  // Initial seed

		vec3 origin = texelFetch(PositionsImage, pixel, 0).xyz;
		vec3 normal = texelFetch(NormalsImage, pixel, 0).xyz;
		float validity = texelFetch(ValidityImage, pixel, 0).x;

		if (validity < 1)
		{
			// return;
		}

		vec3 finalColor = vec3(0);
		vec3 previousColor = vec3(0);

		if (Parameters.AccumulatedSamples != 0)
		{
			previousColor = imageLoad(LightmapOutput, pixel).rgb;
		}

		for (int i = 0; i < Parameters.SamplesPerFrame; i++)
		{
			vec3 direction = RandomDirectionHemisphere(rngState, normal);

			finalColor += PathTrace(origin, direction, Parameters.Bounces, rngState);
		}

		finalColor = finalColor + previousColor;

		if ((Parameters.AccumulatedSamples + Parameters.SamplesPerFrame) >= Parameters.RequestedSamples)
		{
			finalColor = finalColor / Parameters.RequestedSamples;
		}

		imageStore(LightmapOutput, pixel, vec4(finalColor, 1));
	}
#endif

#ifdef MISS_SHADER
	#include "PathTraceMissing.glsl"
#endif

#ifdef CLOSEST_HIT_SHADER
	#include "PathTraceClosestHit.glsl"
#endif