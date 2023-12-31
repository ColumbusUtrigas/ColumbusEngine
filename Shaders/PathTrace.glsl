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

    layout(binding = 0, set = 6) uniform accelerationStructureEXT acc; // TODO
    layout(binding = 1, set = 6, rgba32f) uniform image2D img;

	#include "GPUScene.glsl"
	#include "CommonRayTracing.glsl"

	// TODO: move to a global GPUScene/View cbuffer
    layout(push_constant) uniform params
    {
        vec4 camPos;
        vec4 camDir;
        vec4 camUp;
        vec4 camSide;
        int frameNumber;
        int reset;
		int bounces;
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

        uint rngState = gl_LaunchIDEXT.x * pixel.y + pixel.x * (rayParams.frameNumber + 1);  // Initial seed

        vec3 u = camSide.xyz;
        vec3 v = camUp.xyz;

        const float planeWidth = tan(fov * 0.5f);
        u *= (planeWidth * aspect);
        v *= planeWidth;

        const vec3 rayDir = normalize(camDir.xyz + (u * (uv.x*2-1)) - (v * (uv.y*2-1)));

        // The sum of the colors of all of the samples.
        // vec3 summedPixelColor = vec3(1.0);

        vec3 origin = camPos;
        vec3 direction = rayDir;

        // const uint cullMask = 0xFF;
        // const uint sbtRecordOffset = 0;
        // const uint sbtRecordStride = 0;
        // const uint missIndex = 0;
        // const float tmin = 0.0f;
        // const float tmax = 5000.0f;
        // const int payloadLocation = 0;
        // const int NUM_SAMPLES = 1;

        // Define the field of view by the vertical slope of the topmost rays:
        // const float fovVerticalSlope = 1.0 / 5.0;

		vec3 finalColor = PathTrace(origin, direction, rayParams.bounces, rngState);

        if (rayParams.reset == 1)
        {
            imageStore(img, ivec2(gl_LaunchIDEXT), vec4(0,0,0,1));
        }

        if (rayParams.frameNumber > 1)
        {
            vec4 prevValue = imageLoad(img, ivec2(gl_LaunchIDEXT));
            if (prevValue.rgb != vec3(0))
            {
                finalColor += prevValue.rgb;
            }
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
