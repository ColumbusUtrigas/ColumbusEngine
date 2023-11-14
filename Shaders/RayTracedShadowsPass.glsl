#version 460 core
#extension GL_EXT_ray_tracing : enable
#extension GL_GOOGLE_include_directive : require

struct RayPayload {
	float Distance;
};

#ifdef RAYGEN_SHADER
	layout(location = 0) rayPayloadEXT RayPayload Payload;

	#include "GPUScene.glsl"

	#define GOLDEN_RATIO 1.618033988749894
	#define PI 3.14159265359

	layout(binding = 0, set = 7) uniform accelerationStructureEXT AccelerationStructure; // TODO
	layout(binding = 1, set = 7, rgba16f) uniform image2D ShadowsBuffer;
	layout(binding = 2, set = 7) uniform sampler2D GBufferNormals;
	layout(binding = 3, set = 7) uniform sampler2D GBufferWorldPosition;

	layout(push_constant) uniform params
	{
		vec3 Direction;
		float Angle;
		float Random;
	} Params;

	// 0 < angle < 2pi
	vec3 SampleConeFibonacci(uint i, uint N, float angle)
	{
		float phi = acos(1 - angle * i / (N*PI));
		float theta = 2*PI * i / GOLDEN_RATIO;

		return vec3(cos(theta) * sin(phi), 1, sin(theta) * sin(phi));
		// return vec3(cos(theta) * sin(phi), sin(theta) * sin(phi), cos(phi));
	}

	// Random in range [0-1]
	vec3 SampleConeRay(vec3 Direction, float BaseRadius, vec2 Random)
	{
		// generate points in circle
		float theta = Random.x * 2 * PI;
		float radius = Random.y * 0.5 * BaseRadius;
		vec2 circle = vec2(cos(theta) * radius, sin(theta) * radius);

		// generate cone basis
		// TODO: verify handinness
		vec3 up = Direction.y < 0.999 ? vec3(0, 1, 0) : vec3(0, 0, 1);
		vec3 right = normalize(cross(up, Direction));
		vec3 forward = normalize(cross(right, up));

		// use basis to transform points
		return Direction + circle.x * right + circle.y * forward;
	}

	float rand(vec2 co){
		return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
	}

	void main()
	{
		vec2 uv = vec2(gl_LaunchIDEXT.xy) / vec2(gl_LaunchSizeEXT.xy - 1);

		vec3 origin = texture(GBufferWorldPosition, uv).xyz;

		vec3 direction = SampleConeRay(Params.Direction, Params.Angle, vec2(rand(uv + Params.Random), rand(uv + Params.Random)));
		direction = normalize(direction);

		traceRayEXT(AccelerationStructure, gl_RayFlagsOpaqueEXT | gl_RayFlagsTerminateOnFirstHitEXT,
			0xFF, 0, 0, 0, origin, 1, direction, 5000, 0);

		float Result = 0;
		if (Payload.Distance > 0)
		{
			Result = 0;
		} else
		{
			Result = 1;
		}

		imageStore(ShadowsBuffer, ivec2(gl_LaunchIDEXT), vec4(Result));
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
