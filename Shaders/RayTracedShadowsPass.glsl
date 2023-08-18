#version 460 core
#extension GL_EXT_ray_tracing : enable

struct RayPayload {
	float Distance;
};

#ifdef RAYGEN_SHADER
	layout(location = 0) rayPayloadEXT RayPayload Payload;

	#include <GPUScene>

	#define GOLDEN_RATIO 1.618033988749894
	#define PI 3.14159265359

	layout(binding = 0, set = 7) uniform accelerationStructureEXT AccelerationStructure; // TODO
	layout(binding = 1, set = 7, rgba16f) uniform image2D ShadowsBuffer;
	layout(binding = 2, set = 7) uniform sampler2D GBufferNormals;
	layout(binding = 3, set = 7) uniform sampler2D GBufferWorldPosition;

	// 0 < angle < 2pi
	vec3 SampleConeFibonacci(uint i, uint N, float angle)
	{
		float phi = acos(1 - angle * i / (N*PI));
		float theta = 2*PI * i / GOLDEN_RATIO;

		return vec3(cos(theta) * sin(phi), 1, sin(theta) * sin(phi));
		// return vec3(cos(theta) * sin(phi), sin(theta) * sin(phi), cos(phi));
	}

	float rand(vec2 co){
		return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
	}

	void main()
	{
		vec2 uv = vec2(gl_LaunchIDEXT.xy) / vec2(gl_LaunchSizeEXT.xy - 1);

		vec3 origin = texture(GBufferWorldPosition, uv).xyz;
        vec3 direction = normalize(vec3(1,1,0));
		// direction = normalize(SampleConeFibonacci(gl_LaunchIDEXT.x, gl_LaunchSizeEXT.x, 0.01));

		// float phi = rand(uv) * PI * 0.03;
		// float theta = rand(uv+vec2(0.05)) * PI;
		// direction = vec3(cos(phi) * cos(theta), sin(phi) * cos(theta), sin(theta));

		direction = vec3(rand(uv)*0.01, 1, rand(uv)*0.01);
		direction = normalize(direction);

		// return [r * math.cos(phi) * math.cos(theta), r * math.sin(phi) * math.cos(theta) , r * math.sin(theta)]
		// return spherical_to_cartesian(1, phi, theta);

		// imageStore(ShadowsBuffer, ivec2(gl_LaunchIDEXT), vec4(rand(uv)));
		// return;

		traceRayEXT(AccelerationStructure, gl_RayFlagsOpaqueEXT | gl_RayFlagsTerminateOnFirstHitEXT,
			0xFF, 0, 0, 0, origin, 1, direction, 5000, 0);

		if (Payload.Distance > 0)
		{
			imageStore(ShadowsBuffer, ivec2(gl_LaunchIDEXT), vec4(0));
		} else
		{
			imageStore(ShadowsBuffer, ivec2(gl_LaunchIDEXT), vec4(1));
		}
	}
#endif

#ifdef MISS_SHADER
	layout(location = 0) rayPayloadInEXT RayPayload Payload;

	void main() {
		Payload.Distance = -10;
	}
#endif

#ifdef CLOSEST_HIT_SHADER
	layout(location = 0) rayPayloadInEXT RayPayload Payload;

	void main() {
		Payload.Distance = gl_HitTEXT;
	}
#endif
