#ifndef RADIANCE_CACHE_GLSL
#define RADIANCE_CACHE_GLSL

// must define RADIANCE_CACHE_SET and RADIANCE_CACHE_BINDING before including

struct RadianceCacheEntry
{
	float4 Irradiance; // 16
};

#define NUM_CASCADES 6
#define CASCADE_RESOLUTION 16

[[vk::binding(RADIANCE_CACHE_BINDING, RADIANCE_CACHE_SET)]] RWStructuredBuffer<RadianceCacheEntry> RadianceCacheBuffer;

uint CascadeIndexOffset(uint Cascade)
{
	return Cascade * CASCADE_RESOLUTION * CASCADE_RESOLUTION * CASCADE_RESOLUTION;
}

uint CascadeProbeIndex(int3 Index3d)
{
	return Index3d.x + Index3d.y * CASCADE_RESOLUTION + Index3d.z * CASCADE_RESOLUTION * CASCADE_RESOLUTION;
}

float CalcCascadeProbeSpacing(uint Cascade)
{
	return pow(2, Cascade);
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

float3 SampleRadianceCache(float3 CameraPos, float3 WorldPosition)
{
	const float3 CascadePos = CameraPos;
	const uint Cascade = CalcCascadeFromDistance(distance(CascadePos, WorldPosition));
	//return float3(float(Cascade) / NUM_CASCADES);
	const float ProbeSpacing = CalcCascadeProbeSpacing(Cascade);
	float3 CascadeBounds = ProbeSpacing * float3(NUM_CASCADES, NUM_CASCADES, NUM_CASCADES);

	int3 ProbeIndex = int3((CascadePos - WorldPosition + (CascadeBounds / 2)) / ProbeSpacing);
	ProbeIndex = clamp(ProbeIndex, int3(0,0,0), int3(CASCADE_RESOLUTION-1, CASCADE_RESOLUTION-1, CASCADE_RESOLUTION-1));
	const uint ProbeFlatIndex = CascadeIndexOffset(Cascade) + CascadeProbeIndex(ProbeIndex);

	return RadianceCacheBuffer[ProbeFlatIndex].Irradiance.rgb;
}

#endif