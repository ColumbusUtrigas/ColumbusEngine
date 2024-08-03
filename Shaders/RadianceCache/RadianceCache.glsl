#ifndef RADIANCE_CACHE_GLSL
#define RADIANCE_CACHE_GLSL

// must define RADIANCE_CACHE_SET and RADIANCE_CACHE_BINDING before including

struct RadianceCacheEntry
{
	vec4 Irradiance; // 16
};

#define NUM_CASCADES 6
#define CASCADE_RESOLUTION 16

layout(binding = RADIANCE_CACHE_BINDING, set = RADIANCE_CACHE_SET) buffer RadianceCacheBuffer {
	RadianceCacheEntry Probes[];
} RadianceCache;

uint CascadeIndexOffset(uint Cascade)
{
	return Cascade * CASCADE_RESOLUTION * CASCADE_RESOLUTION * CASCADE_RESOLUTION;
}

uint CascadeProbeIndex(ivec3 Index3d)
{
	return Index3d.x + Index3d.y * CASCADE_RESOLUTION + Index3d.z * CASCADE_RESOLUTION * CASCADE_RESOLUTION;
}

float CalcCascadeProbeSpacing(uint Cascade)
{
	return pow(2, Cascade);
	//return float(Cascade + 1); // TODO: improve
}

uint CalcCascadeFromDistance(float Distance)
{
	int Cascade = 0;
	for (int i = 0; i < NUM_CASCADES; i++, Cascade++)
	{
		if (CalcCascadeProbeSpacing(i) * (CASCADE_RESOLUTION / 2) > Distance)
			return Cascade;
	}

	return Cascade;
	//return uint(floor(max(log2(Distance / CASCADE_RESOLUTION * 2), 0)));
	//float(x + 1)* CASCADE_RESOLUTION / 2 < dist
}

vec3 SampleRadianceCache(vec3 CameraPos, vec3 WorldPosition)
{
	const vec3 CascadePos = CameraPos;
	const uint Cascade = CalcCascadeFromDistance(distance(CascadePos, WorldPosition));
	//return vec3(float(Cascade) / NUM_CASCADES);
	const float ProbeSpacing = CalcCascadeProbeSpacing(Cascade);
	vec3 CascadeBounds = ProbeSpacing * vec3(NUM_CASCADES);

	ivec3 ProbeIndex = ivec3((CascadePos - WorldPosition + (CascadeBounds / 2)) / ProbeSpacing);
	ProbeIndex = clamp(ProbeIndex, ivec3(0), ivec3(CASCADE_RESOLUTION-1));
	const uint ProbeFlatIndex = CascadeIndexOffset(Cascade) + CascadeProbeIndex(ProbeIndex);

	return RadianceCache.Probes[ProbeFlatIndex].Irradiance.rgb;
}

#endif