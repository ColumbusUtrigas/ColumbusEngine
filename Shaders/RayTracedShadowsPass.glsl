#version 460 core
#extension GL_EXT_ray_tracing : enable

struct RayPayload {
	float Distance;
};

#ifdef RAYGEN_SHADER
	layout(location = 0) rayPayloadEXT RayPayload Payload;

	#include <GPUScene>

	layout(binding = 0, set = 7) uniform accelerationStructureEXT AccelerationStructure; // TODO
	layout(binding = 1, set = 7, rgba16f) uniform image2D ShadowsBuffer;
	layout(binding = 2, set = 7) uniform sampler2D GBufferNormals;
	layout(binding = 3, set = 7) uniform sampler2D GBufferWorldPosition;

	void main()
	{
		vec2 uv = vec2(gl_LaunchIDEXT.xy) / vec2(gl_LaunchSizeEXT.xy - 1);

		vec3 origin = texture(GBufferWorldPosition, uv).xyz;
        vec3 direction = normalize(vec3(1,1,0));

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
