// Baked probe volume, prototype format:
// each probe stores an HL2-style ambient cube in six directional lobes.

struct IrradianceProbe
{
    // xyz: current probe world position after relocation, w: active state.
    float4 PositionState;
    float4 Irradiance[6];
    // DDGI-style octahedral visibility moments. x: mean / 2, y: meanSq / 2.
    float2 Distance[64];
};

struct IrradianceVolume
{
    float3 Position;
    float3 Extent;
    int3   ProbesCount;
    int    ProbeBufferIndex;
    float  NormalBias;
    float  BlendDistance;
    float  Priority;
};

static const float3 IRRADIANCE_BASIS[6] =
{
    float3( 1.0,  0.0,  0.0),
    float3( 0.0,  1.0,  0.0),
    float3( 0.0,  0.0,  1.0),
    float3(-1.0,  0.0,  0.0),
    float3( 0.0, -1.0,  0.0),
    float3( 0.0,  0.0, -1.0)
};

#ifndef IRRADIANCE_PROBES_SET
#define IRRADIANCE_PROBES_SET 0
#endif

#ifdef IRRADIANCE_DECLARE_PROBE_STORAGE
IRRADIANCE_DECLARE_PROBE_STORAGE
#else
[[vk::binding(0, IRRADIANCE_PROBES_SET)]] RWStructuredBuffer<IrradianceProbe> IrradianceProbes;
#define IRRADIANCE_LOAD_PROBE(Index) IrradianceProbes[Index]
#endif

struct IrradianceFixedRayData
{
    // x: signed hit distance. Positive frontface, negative shortened backface, large positive miss.
    float Distance;
};

float3 IrradianceSphericalFibonacci(int SampleIndex, int SampleCount)
{
    float Phi = 2.39996322972865332 * SampleIndex;
    float CosTheta = 1.0 - (2.0 * (SampleIndex + 0.5) / max(SampleCount, 1));
    float SinTheta = sqrt(saturate(1.0 - CosTheta * CosTheta));
    return float3(cos(Phi) * SinTheta, sin(Phi) * SinTheta, CosTheta);
}

float3 IrradianceFixedRayDirection(int RayIndex)
{
    return IrradianceSphericalFibonacci(RayIndex, 32);
}

float2 IrradianceSignNotZero(float2 V)
{
    return float2(V.x >= 0.0 ? 1.0 : -1.0, V.y >= 0.0 ? 1.0 : -1.0);
}

float2 IrradianceOctEncode(float3 Direction)
{
    Direction /= max(abs(Direction.x) + abs(Direction.y) + abs(Direction.z), 0.00001);
    float2 Encoded = Direction.xy;
    if (Direction.z < 0.0)
    {
        Encoded = (1.0 - abs(Encoded.yx)) * IrradianceSignNotZero(Encoded);
    }
    return Encoded * 0.5 + 0.5;
}

float3 IrradianceOctDecode(float2 Encoded)
{
    float2 F = Encoded * 2.0 - 1.0;
    float3 Direction = float3(F.x, F.y, 1.0 - abs(F.x) - abs(F.y));
    if (Direction.z < 0.0)
    {
        Direction.xy = (1.0 - abs(Direction.yx)) * IrradianceSignNotZero(Direction.xy);
    }
    return normalize(Direction);
}

int IrradianceDistanceTexelIndex(int2 Texel)
{
    Texel = clamp(Texel, int2(0, 0), int2(7, 7));
    return Texel.y * 8 + Texel.x;
}

float3 IrradianceDistanceTexelDirection(int Index)
{
    int2 Texel = int2(Index % 8, Index / 8);
    return IrradianceOctDecode((float2(Texel) + 0.5) / 8.0);
}

float3 IrradianceProbeSpacing(IrradianceVolume Volume)
{
    return Volume.Extent / max((float3)Volume.ProbesCount, float3(1, 1, 1));
}

float IrradianceVolumeMinProbeSpacing(IrradianceVolume Volume)
{
    float3 Spacing = IrradianceProbeSpacing(Volume);
    return max(0.001, min(Spacing.x, min(Spacing.y, Spacing.z)));
}

int IrradianceProbeGridIdToLinear(IrradianceVolume Volume, int3 ProbeId)
{
    ProbeId = clamp(ProbeId, int3(0, 0, 0), Volume.ProbesCount - 1);
    return ProbeId.x * (Volume.ProbesCount.y * Volume.ProbesCount.z) + ProbeId.y * Volume.ProbesCount.z + ProbeId.z;
}

float IrradianceVolumeBlendWeight(IrradianceVolume Volume, float3 Position)
{
    float3 HalfExtent = max(Volume.Extent * 0.5, float3(0.001, 0.001, 0.001));
    float3 Delta = abs(Position - Volume.Position) - HalfExtent;
    float BlendDistance = Volume.BlendDistance > 0.0 ? Volume.BlendDistance : IrradianceVolumeMinProbeSpacing(Volume);
    float3 AxisWeight = 1.0 - saturate(Delta / max(BlendDistance, 0.001));
    return AxisWeight.x * AxisWeight.y * AxisWeight.z;
}

float IrradianceVolumePriorityWeight(IrradianceVolume Volume)
{
    if (Volume.Priority > 0.0)
    {
        return Volume.Priority;
    }

    return 1.0 / IrradianceVolumeMinProbeSpacing(Volume);
}

float3 SampleIrradianceProbe(IrradianceProbe Probe, float3 Normal)
{
    Normal = normalize(Normal);

    float3 NormalSquared = Normal * Normal;
    return Probe.Irradiance[Normal.x >= 0.0 ? 0 : 3].rgb * NormalSquared.x +
           Probe.Irradiance[Normal.y >= 0.0 ? 1 : 4].rgb * NormalSquared.y +
           Probe.Irradiance[Normal.z >= 0.0 ? 2 : 5].rgb * NormalSquared.z;
}

float2 SampleIrradianceProbeDistance(IrradianceProbe Probe, float3 Direction)
{
    float2 Uv = IrradianceOctEncode(normalize(Direction));
    float2 Texel = Uv * 8.0 - 0.5;
    int2 Base = (int2)floor(Texel);
    float2 Frac = saturate(Texel - Base);

    float2 Moments = float2(0.0, 0.0);
    [unroll]
    for (int y = 0; y <= 1; y++)
    {
        [unroll]
        for (int x = 0; x <= 1; x++)
        {
            int2 Offset = int2(x, y);
            float2 Weight2 = lerp(1.0 - Frac, Frac, (float2)Offset);
            Moments += Probe.Distance[IrradianceDistanceTexelIndex(Base + Offset)] * (Weight2.x * Weight2.y);
        }
    }

    return 2.0 * Moments;
}

float3 SampleIrradianceProbes(IrradianceVolume Volume, float3 Position, float3 Normal)
{
    if (IrradianceVolumeBlendWeight(Volume, Position) <= 0.0)
    {
        return float3(0.0, 0.0, 0.0);
    }
    
    float3 Spacing = IrradianceProbeSpacing(Volume);
    float3 BiasedPosition = Position + normalize(Normal) * (Volume.NormalBias * min(Spacing.x, min(Spacing.y, Spacing.z)));
    float3 Local01 = saturate((BiasedPosition - (Volume.Position - Volume.Extent * 0.5)) / max(Volume.Extent, float3(0.001, 0.001, 0.001)));
    float3 ProbeGrid = Local01 * Volume.ProbesCount - float3(0.5, 0.5, 0.5);
    int3 BaseId = (int3)floor(ProbeGrid);
    float3 Frac = saturate(ProbeGrid - BaseId);

    float3 Colour = float3(0.0, 0.0, 0.0);
    float SumWeight = 0.0;

    [unroll]
    for (int x = 0; x <= 1; x++)
    {
        [unroll]
        for (int y = 0; y <= 1; y++)
        {
            [unroll]
            for (int z = 0; z <= 1; z++)
            {
                int3 Offset = int3(x, y, z);
                int3 ProbeId = clamp(BaseId + Offset, int3(0, 0, 0), Volume.ProbesCount - 1);
                float3 Tri = lerp(float3(1.0, 1.0, 1.0) - Frac, Frac, (float3)Offset);
                float Weight = Tri.x * Tri.y * Tri.z;

                IrradianceProbe Probe = IRRADIANCE_LOAD_PROBE(IrradianceProbeGridIdToLinear(Volume, ProbeId));
                if (Probe.PositionState.w < 0.5)
                {
                    continue;
                }

                float3 ProbeToPosition = BiasedPosition - Probe.PositionState.xyz;
                float ProbeDistance = length(ProbeToPosition);
                float2 FilteredDistance = SampleIrradianceProbeDistance(Probe, ProbeToPosition);
                float Variance = abs(FilteredDistance.x * FilteredDistance.x - FilteredDistance.y);

                float Visibility = 1.0;
                if (ProbeDistance > FilteredDistance.x)
                {
                    float V = ProbeDistance - FilteredDistance.x;
                    Visibility = Variance / (Variance + V * V);
                    Visibility = max(Visibility * Visibility * Visibility, 0.0);
                }
                
                const float VisibilityFloor = 0.2; // 0.05 DDGI

                float3 ProbeDirection = normalize(Probe.PositionState.xyz - Position);
                float WrapShading = (dot(ProbeDirection, normalize(Normal)) + 1.0) * 0.5;
                Weight *= ((WrapShading * WrapShading) + 0.2) * max(VisibilityFloor, Visibility);
                Weight = max(Weight, 0.000001);

                const float CrushThreshold = 0.2;
                if (Weight < CrushThreshold)
                {
                    //Weight *= (Weight * Weight) / (CrushThreshold * CrushThreshold);
                }

                Colour += SampleIrradianceProbe(Probe, Normal) * Weight;
                SumWeight += Weight;
            }
        }
    }

    return SumWeight > 0.0 ? Colour / SumWeight : float3(0.0, 0.0, 0.0);
}
