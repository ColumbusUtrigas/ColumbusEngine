// HL2 irradiance cube
struct IrradianceProbe
{
	vec3 Position; // 16
	// vec3 test;
	vec3 Irradiance[6]; // 112

	// vec3 FullIrradiance[64];
	// vec3 Directions[64];
};

struct IrradianceVolume
{
	vec3 Position;
	vec3 Extent;
	ivec3 ProbesCount;
};

const vec3 IRRADIANCE_BASIS[6] = { vec3( 1.0, 0.0, 0.0 ),
                                   vec3( 0.0, 1.0, 0.0 ),
                                   vec3( 0.0, 0.0, 1.0 ),
                                   vec3( -1., 0.0, 0.0 ),
                                   vec3( 0.0, -1., 0.0 ),
                                   vec3( 0.0, 0.0, -1. ) };

#ifndef IRRADIANCE_PROBES_SET
#define IRRADIANCE_PROBES_SET 0
#endif

layout(binding = 0, set = IRRADIANCE_PROBES_SET) buffer IrradianceProbesBuffer {
	IrradianceProbe Probes[];
} IrradianceProbes;

int IrradianceProbeGridIdToLinear(IrradianceVolume Volume, ivec3 ProbeId)
{
	ProbeId = clamp(ProbeId, ivec3(0), Volume.ProbesCount - 1);
	return ProbeId.x*(Volume.ProbesCount.y*Volume.ProbesCount.z) + ProbeId.y + ProbeId.z*Volume.ProbesCount.y;
}

IrradianceProbe GetClosestIrradianceProbe(IrradianceVolume Volume, vec3 Position)
{
	vec3 CellSize = Volume.Extent / Volume.ProbesCount;
	Position -= Volume.Position;
	Position += Volume.Extent / 2;
	ivec3 ProbeId = ivec3(Position / CellSize);
	ProbeId = clamp(ProbeId, ivec3(0), Volume.ProbesCount - 1);
	// int Id = clamp(ProbeId.x, 0, 4)*20 + clamp(ProbeId.y, 0, 3)*5 + clamp(ProbeId.z, 0, 4);
	int Id = ProbeId.x*Volume.ProbesCount.y*Volume.ProbesCount.z + ProbeId.y*Volume.ProbesCount.z + ProbeId.z;
	// int Id = IrradianceProbeGridIdToLinear(Volume, ProbeId);

	return IrradianceProbes.Probes[Id];
}

vec3 SampleIrradianceProbe(IrradianceProbe Probe, vec3 Normal)
{
	vec3 Colour = vec3(0);
	for (int i = 0; i < 6; i++)
	{
		Colour += Probe.Irradiance[i].rgb * (1-max(dot(IRRADIANCE_BASIS[i], Normal), 0));
	}
	return Colour / 6.0;
}

vec3 SampleIrradianceProbes(IrradianceVolume Volume, vec3 Position, vec3 Normal)
{
	vec3 Colour = vec3(0);
	IrradianceProbe Probe = GetClosestIrradianceProbe(Volume, Position);
	Colour = SampleIrradianceProbe(Probe, Normal);

	// sampling neighbours
	vec3 PosRelativeToProbe = Position - Probe.Position;
	vec3 CellSize = Volume.Extent / Volume.ProbesCount;
	vec3 NextProbes = sign(PosRelativeToProbe) * CellSize;

	IrradianceProbe Neighbour1 = GetClosestIrradianceProbe(Volume, Position + NextProbes.x * vec3(1, 0, 0));
	IrradianceProbe Neighbour2 = GetClosestIrradianceProbe(Volume, Position + NextProbes.y * vec3(0, 1, 0));
	IrradianceProbe Neighbour3 = GetClosestIrradianceProbe(Volume, Position + NextProbes.z * vec3(0, 0, 1));

	vec3 LerpFactors = abs(PosRelativeToProbe) / CellSize;
	Colour = mix(Colour, SampleIrradianceProbe(Neighbour1, Normal), LerpFactors.x);
	Colour = mix(Colour, SampleIrradianceProbe(Neighbour2, Normal), LerpFactors.y);
	Colour = mix(Colour, SampleIrradianceProbe(Neighbour3, Normal), LerpFactors.z);

	// TODO: check if outside and return something invalid

	return Colour;
}
