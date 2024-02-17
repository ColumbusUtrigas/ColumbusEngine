#version 460 core
#extension GL_EXT_ray_tracing : enable
#extension GL_GOOGLE_include_directive : require

struct RayPayload {
	vec4 colorAndDist;
	vec4 normalAndObjId;
};

#ifdef RAYGEN_SHADER
	layout(location = 0) rayPayloadEXT RayPayload Payload;
	layout(location = 1) rayPayloadEXT RayPayload shadowPayload; // TODO:

	layout(binding = 0, set = 2) uniform accelerationStructureEXT acc; // TODO:
	layout(binding = 1, set = 2, rgba16f) uniform image2D ResultReflections;
	layout(binding = 2, set = 2) uniform sampler2D GBufferNormals;
	layout(binding = 3, set = 2) uniform sampler2D GBufferWorldPosition;
	layout(binding = 4, set = 2) uniform sampler2D GBufferRoughnessMetallic;
	
	#include "../GPUScene.glsl" // TODO:
	#include "../BRDF.glsl"
	#include "../Common.glsl"

	layout(push_constant) uniform params
	{
		vec4 CameraPosition;
		uint Random;
	} Params;

	void main()
	{
		const uvec2 pixel = gl_LaunchIDEXT.xy;
		const vec2 uv = vec2(gl_LaunchIDEXT.xy) / vec2(gl_LaunchSizeEXT.xy - 1);

		uint RngState = gl_LaunchIDEXT.x * pixel.y + pixel.x * (Params.Random + 1);  // Initial seed

		vec3 Origin = texture(GBufferWorldPosition, uv).xyz;
		vec3 Normal = texture(GBufferNormals, uv).xyz;
		vec3 Direction = normalize(Origin - Params.CameraPosition.xyz);

		BRDFData BRDF;
		BRDF.N = Normal;
		BRDF.V = -Direction;
		BRDF.Albedo = vec3(1); // TODO: remove?
		BRDF.Roughness = 0.3; // TODO: materials
		BRDF.Metallic = 0.0; // TODO: materials

		Direction = reflect(Direction, Normal);
		Direction = RandomDirectionGGX(BRDF.Roughness*BRDF.Roughness, Direction, UniformDistrubition2d(RngState));
		BRDF.L = Direction;

		float MaxDistance = 5000; // TODO: make it a parameter

		traceRayEXT(acc, gl_RayFlagsOpaqueEXT,
			0xFF, 0, 0, 0, Origin, 0.01, Direction, MaxDistance, 0);

		// TODO: evaluate lighting at the hit point
		vec3 Sample = Payload.colorAndDist.rgb;

		vec3 FinalResult = EvaluateBRDF(BRDF, Sample);

		imageStore(ResultReflections, ivec2(gl_LaunchIDEXT), vec4(FinalResult, 1));
		//imageStore(ResultReflections, ivec2(gl_LaunchIDEXT), vec4(Direction, 1));
	}
#endif

#ifdef MISS_SHADER
	#include "../PathTraceMissing.glsl"
#endif

#ifdef CLOSEST_HIT_SHADER
	#include "../PathTraceClosestHit.glsl"
#endif