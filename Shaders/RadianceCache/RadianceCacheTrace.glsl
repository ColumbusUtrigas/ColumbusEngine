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

	#define RADIANCE_CACHE_BINDING 1
	#define RADIANCE_CACHE_SET 2
	#include "RadianceCache.glsl"

	layout(binding = 0, set = 2) uniform accelerationStructureEXT acc;

	#include "../GPUScene.glsl"
	#include "../CommonRayTracing.glsl"

	layout(push_constant) uniform params
	{
		vec3 CameraPosition;
		int Random;
	} Params;

	ivec3 Probe3dIndex(uint FlatId)
	{
		return ivec3(
			FlatId % CASCADE_RESOLUTION,
			(FlatId / CASCADE_RESOLUTION) % CASCADE_RESOLUTION,
			FlatId / (CASCADE_RESOLUTION * CASCADE_RESOLUTION)
		);
	}

	vec3 CalcProbePosition(float ProbeSpacing, ivec3 ProbeIndex)
	{
		vec3 CascadeBounds = ProbeSpacing * vec3(NUM_CASCADES);
		return ProbeIndex * ProbeSpacing + Params.CameraPosition - (CascadeBounds / 2);
	}

	void main()
	{
		uint rngState = gl_LaunchIDEXT.x + gl_LaunchIDEXT.y + Params.Random;  // Initial seed

		const uint Cascade = gl_LaunchIDEXT.x;
		const uint Probe   = gl_LaunchIDEXT.y;

		const float ProbeSpacing  = CalcCascadeProbeSpacing(Cascade);
		const vec3  CascadeCenter = Params.CameraPosition; // TODO: improve
		const ivec3 ProbeIndex    = Probe3dIndex(Probe);
		const uint  ProbeFlatId   = CascadeIndexOffset(Cascade) + Probe;
		const vec3  ProbePosition = CalcProbePosition(ProbeSpacing, ProbeIndex);

		const vec3 Direction = RandomDirectionSphere(UniformDistrubition2d(rngState));

		const int Bounces = 5;
		const vec3 Radiance = PathTrace(ProbePosition, Direction, Bounces, rngState);
		RadianceCache.Probes[ProbeFlatId].Irradiance = mix(RadianceCache.Probes[ProbeFlatId].Irradiance, vec4(Radiance, 1), 0.1);
	}
#endif

#ifdef MISS_SHADER
	//#include "../PathTraceMissing.glsl"
layout(location = 0) rayPayloadInEXT RayPayload payload;

void main() {
	//payload.colorAndDist = vec4(0.412f, 0.796f, 1.0f, -1);
	payload.colorAndDist = vec4(0, 0, 0, -1); // ignore sky
	payload.normalAndObjId = vec4(0, 0, 0, 0);
	payload.RoughnessMetallic = vec2(1, 0);
}
#endif

#ifdef CLOSEST_HIT_SHADER
	#include "../PathTraceClosestHit.glsl"
#endif