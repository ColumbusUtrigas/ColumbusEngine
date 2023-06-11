// HL2 irradiance cube
struct IrradianceProbe
{
	vec3 Position; // 16
	vec3 test;
	vec3 Irradiance[6]; // 112

	vec3 FullIrradiance[64];
	vec3 Directions[64];
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

IrradianceProbe GetClosestIrradianceProbe(IrradianceVolume Volume, vec3 Position)
{
	vec3 CellSize = Volume.Extent / Volume.ProbesCount;
	Position -= Volume.Position;
	ivec3 CubeId = ivec3(Position / CellSize);
	int Id = clamp(CubeId.x, 0, 4)*20 + clamp(CubeId.y, 0, 3) + clamp(CubeId.z, 0, 4)*4;

	return IrradianceProbes.Probes[Id];
}

vec3 SampleIrradianceProbes(IrradianceVolume Volume, vec3 Position, vec3 Normal)
{
	vec3 Color = vec3(0);
	IrradianceProbe Probe = GetClosestIrradianceProbe(Volume, Position);

	for (int i = 0; i < 6; i++)
	{
		Color += Probe.Irradiance[i].rgb * (1-max(dot(IRRADIANCE_BASIS[i], Normal), 0));
	}

	Color /= 6.0;

	return Color;
}
