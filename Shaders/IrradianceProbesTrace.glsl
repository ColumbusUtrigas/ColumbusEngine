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

	#define IRRADIANCE_PROBES_SET 7
	#include "IrradianceProbeCommon.glsl"

	layout(binding = 0, set = 6) uniform accelerationStructureEXT acc;
	// IRRADIANCE_PROBES_BUFFER(8) // set 8

	#include "GPUScene.glsl"
	#include "CommonRayTracing.glsl"

	layout(push_constant) uniform params
	{
		int SamplesPerProbe;
		int Bounces;
		int Random;
	} Params;

	#define ACCESS_PROBE() IrradianceProbes.Probes[gl_LaunchIDEXT.x]

	void main()
	{
		uint rngState = gl_LaunchIDEXT.x + gl_LaunchIDEXT.y + Params.Random;  // Initial seed

		vec3 origin = ACCESS_PROBE().Position;

		for (int i = 0; i < 6; i++)
		{
			ACCESS_PROBE().Irradiance[i] = vec3(0);
			// ACCESS_PROBE().Irradiance[i] = PathTrace(origin, IRRADIANCE_BASIS[i], Params.Bounces, rngState);
		}

		for (int i = 0; i < 6; i++)
		{
			for (int s = 0; s < Params.SamplesPerProbe; s++)
			{
				// uint asd = s;
				// vec3 direction = RandomDirectionSphere(asd);
				vec3 direction = RandomDirectionHemisphere(rngState, IRRADIANCE_BASIS[i]);
				// vec3 direction = RandomDirectionSphere(rngState);
				vec3 color = PathTrace(origin, direction, Params.Bounces, rngState);

				// IrradianceProbes.Probes[0].test += color / 128.0;
				// ACCESS_PROBE().test += color * max(dot(direction, IRRADIANCE_BASIS[0]), 0);
				// ACCESS_PROBE().Directions[s] = direction;
				// ACCESS_PROBE().FullIrradiance[s] = color;

				ACCESS_PROBE().Irradiance[i] += color;
				// ACCESS_PROBE().Irradiance[i] += color * (max(dot(direction, IRRADIANCE_BASIS[i]), 0)) / Params.SamplesPerProbe;
			}
			ACCESS_PROBE().Irradiance[i] /= Params.SamplesPerProbe;
		}

		for (int i = 0; i < 6; i++)
		{
			// IrradianceProbes.Probe.Irradiance[i] = IrradianceProbes.Probe.Irradiance[i] / Params.SamplesPerProbe;
		}
	}
#endif

#ifdef MISS_SHADER
	#include "PathTraceMissing.glsl"
#endif

#ifdef CLOSEST_HIT_SHADER
	#include "PathTraceClosestHit.glsl"
#endif
