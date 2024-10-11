// ROADMAP
// - HL2 ambient cube
// - SH2 simple irradiance volume
// - SH2 aided with volumetric depth test (look up DDGI)
// - ZH3 (look up ZH3 paper from 2024)
// - multiple volumes, or a single volume with some spatial hashing?

// HL2 irradiance probe - stores irradiance by 6 directions
struct IrradianceProbe
{
    float3 Position; // 16
    float3 Irradiance[6]; // 112
};

struct IrradianceVolume
{
    float3 Position;
    float3 Extent;
    int3   ProbesCount;
};

static const float3 IRRADIANCE_BASIS[6] = { float3( 1.0, 0.0, 0.0 ),
                                     float3(0.0, 1.0, 0.0),
                                     float3(0.0, 0.0, 1.0),
                                     float3(-1., 0.0, 0.0),
                                     float3(0.0, -1., 0.0),
                                     float3( 0.0, 0.0, -1. ) };

#ifndef IRRADIANCE_PROBES_SET
#define IRRADIANCE_PROBES_SET 0
#endif

// TODO: multiple volumes
[[vk::binding(0, IRRADIANCE_PROBES_SET)]] RWStructuredBuffer<IrradianceProbe> IrradianceProbes;

int IrradianceProbeGridIdToLinear(IrradianceVolume Volume, int3 ProbeId)
{
    ProbeId = clamp(ProbeId, int3(0,0,0), Volume.ProbesCount - 1);
    return ProbeId.x * (Volume.ProbesCount.y * Volume.ProbesCount.z) + ProbeId.y + ProbeId.z * Volume.ProbesCount.y;
}

int3 GetClosestIrradianceProbeId3d(IrradianceVolume Volume, float3 Position)
{
    float3 CellSize = Volume.Extent / Volume.ProbesCount;
    Position -= Volume.Position;
    Position += Volume.Extent / 2;
    int3 ProbeId = int3(Position / CellSize);
    return clamp(ProbeId, int3(0, 0, 0), Volume.ProbesCount - 1);
}

IrradianceProbe GetClosestIrradianceProbe(IrradianceVolume Volume, float3 Position)
{
    float3 CellSize = Volume.Extent / Volume.ProbesCount;
    Position -= Volume.Position;
    Position += Volume.Extent / 2;
    int3 ProbeId = int3(Position / CellSize);
    ProbeId = clamp(ProbeId, int3(0,0,0), Volume.ProbesCount - 1);
	// int Id = clamp(ProbeId.x, 0, 4)*20 + clamp(ProbeId.y, 0, 3)*5 + clamp(ProbeId.z, 0, 4);
    int Id = ProbeId.x * Volume.ProbesCount.y * Volume.ProbesCount.z + ProbeId.y * Volume.ProbesCount.z + ProbeId.z;
	// int Id = IrradianceProbeGridIdToLinear(Volume, ProbeId);

    return IrradianceProbes[Id];
}

float3 SampleIrradianceProbe(IrradianceProbe Probe, float3 Normal)
{
    float3 Colour = float3(0, 0, 0);
    for (int i = 0; i < 6; i++)
    {
        Colour += Probe.Irradiance[i].rgb * (1 - max(dot(IRRADIANCE_BASIS[i], Normal), 0));
    }
    return Colour / 6.0;
}

float3 SampleIrradianceProbes(IrradianceVolume Volume, float3 Position, float3 Normal)
{
    float3 Colour = float3(0, 0, 0);
    IrradianceProbe Probe = GetClosestIrradianceProbe(Volume, Position);
    Colour = SampleIrradianceProbe(Probe, Normal);

	// sampling neighbours
    float3 PosRelativeToProbe = Position - Probe.Position;
    float3 CellSize = Volume.Extent / Volume.ProbesCount;
    float3 NextProbes = sign(PosRelativeToProbe) * CellSize;
    
    Colour = float3(0, 0, 0);
    
    int3 ProbeId3d = GetClosestIrradianceProbeId3d(Volume, Position);
    
    const int3 Offsets[] =
    {
        int3(0, 0, 0),
        
        int3(+1, 0, 0),
        int3(-1, 0, 0),
        int3(0, +1, 0),
        int3(0, -1, 0),
        int3(0, 0, +1),
        int3(0, 0, -1),
    };

    int totalProbes = Volume.ProbesCount.x * Volume.ProbesCount.y * Volume.ProbesCount.z;
    
    float SumW = 0.0f;
    for (int i = 0; i < totalProbes; i++)
    {
        int3 NeighbourId = clamp(ProbeId3d + Offsets[i], int3(0, 0, 0), Volume.ProbesCount - 1);
        int Id = NeighbourId.x * Volume.ProbesCount.y * Volume.ProbesCount.z + NeighbourId.y * Volume.ProbesCount.z + NeighbourId.z;
        Id = i;
        
        IrradianceProbe Neighbour = IrradianceProbes[Id];

        //IrradianceProbe Neighbour = GetClosestIrradianceProbe(Volume, Position + Offsets[i]*CellSize);

        float W = 1 - distance(Position / CellSize, Neighbour.Position / CellSize);
        Colour += SampleIrradianceProbe(Neighbour, -Normal) * W;
        //SumW += W;
        SumW += 1;
    }
    
    Colour /= SumW;
    return Colour;

    IrradianceProbe Neighbour1 = GetClosestIrradianceProbe(Volume, Position + NextProbes.x * float3(1, 0, 0));
    IrradianceProbe Neighbour2 = GetClosestIrradianceProbe(Volume, Position + NextProbes.y * float3(0, 1, 0));
    IrradianceProbe Neighbour3 = GetClosestIrradianceProbe(Volume, Position + NextProbes.z * float3(0, 0, 1));

    float3 LerpFactors = abs(PosRelativeToProbe) / CellSize;
    Colour = lerp(Colour, SampleIrradianceProbe(Neighbour1, Normal), LerpFactors.x);
    Colour = lerp(Colour, SampleIrradianceProbe(Neighbour2, Normal), LerpFactors.y);
    Colour = lerp(Colour, SampleIrradianceProbe(Neighbour3, Normal), LerpFactors.z);

	// TODO: check if outside and return something invalid

    return Colour;
}
