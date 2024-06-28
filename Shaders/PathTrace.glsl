#version 460 core
#extension GL_EXT_ray_tracing : enable
#extension GL_GOOGLE_include_directive : require

struct RayPayload {
	vec4 colorAndDist;
	vec4 normalAndObjId;
	vec2 RoughnessMetallic;
};

#ifdef RAYGEN_SHADER
	layout(location = 0) rayPayloadEXT RayPayload payload;
	layout(location = 1) rayPayloadEXT RayPayload shadowPayload;

	layout(binding = 0, set = 2) uniform accelerationStructureEXT acc; // TODO
	layout(binding = 1, set = 2, rgba32f) uniform image2D img;
	layout(binding = 2, set = 2, rgba32f) uniform image2D History;

	#include "GPUScene.glsl"
	#include "CommonRayTracing.glsl"

	// TODO: move to a global GPUScene/View cbuffer
	layout(push_constant) uniform params
	{
		vec4 camPos;
		vec4 camDir;
		vec4 camUp;
		vec4 camSide;
		int randomNumber;
		int frameNumber;
		int reset;
		int bounces;
		int hasHistory;
	} rayParams;

	void main() {
		vec3 camPos = rayParams.camPos.xyz;
		vec3 camDir = rayParams.camDir.xyz;
		vec3 camUp = rayParams.camUp.xyz;
		vec3 camSide = rayParams.camSide.xyz;

		const float fov = 45;
		const float aspect = float(gl_LaunchSizeEXT.x) / float(gl_LaunchSizeEXT.y);

		const uvec2 pixel = gl_LaunchIDEXT.xy;
		const vec2 uv = vec2(gl_LaunchIDEXT.xy) / vec2(gl_LaunchSizeEXT.xy - 1);

		// uint rngState = gl_LaunchIDEXT.x * pixel.y + pixel.x * (rayParams.randomNumber + 1);  // Initial seed
		uint rngState = hash(hash(pixel.x) + hash(pixel.y) + (rayParams.randomNumber)); // Initial seed

		vec3 u = camSide.xyz;
		vec3 v = camUp.xyz;

		const float planeWidth = tan(fov * 0.5f);
		u *= (planeWidth * aspect);
		v *= planeWidth;

		const vec3 rayDir = normalize(camDir.xyz + (u * (uv.x*2-1)) - (v * (uv.y*2-1)));

		vec3 origin = camPos;
		vec3 direction = rayDir;

		// random sub-pixel jittering, anti-aliasing
		origin += u * (stepAndOutputRNGFloat(rngState) * 2 - 1) / float(gl_LaunchSizeEXT.x);
		origin += v * (stepAndOutputRNGFloat(rngState) * 2 - 1) / float(gl_LaunchSizeEXT.y);

		vec3 sampleColor = PathTrace(origin, direction, rayParams.bounces, rngState);
		vec3 finalColor = vec3(0);

		int reset = rayParams.reset;
		// reset = 1;

		if (reset == 1)
		{
			finalColor = sampleColor;
		}

		if (reset == 0)
		{
			vec3 previous = imageLoad(History, ivec2(gl_LaunchIDEXT)).rgb;

			float sampleGray = dot(sampleColor, vec3(0.2126, 0.7152, 0.0722));;
			float previousGray = dot(previous, vec3(0.2126, 0.7152, 0.0722));;

			float factor = 1.0 / float(rayParams.frameNumber);
			factor *= max(1, sampleGray - previousGray + 1);
			finalColor = mix(previous, sampleColor, factor);
		}

		imageStore(img, ivec2(gl_LaunchIDEXT), vec4(finalColor, 1));
	}
#endif

// TODO: common miss shader
#ifdef MISS_SHADER
	#include "PathTraceMissing.glsl"
#endif

// TODO: common closest hit shader
#ifdef CLOSEST_HIT_SHADER
	#include "PathTraceClosestHit.glsl"
#endif
