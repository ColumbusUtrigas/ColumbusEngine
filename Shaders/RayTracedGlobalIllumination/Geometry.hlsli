#pragma once

#include "../Common.hlsli"

struct RTGIGeometryConstants
{
    float4x4 InverseViewProjection;
    float4x4 PreviousInverseViewProjection;
    float4 CameraPositionPixelSize;
    float4 JitterDelta;
    float4 DepthProjection; // Current A/B, then previous A/B: depth = B / viewZ - A.
    int4 Sizes;  // Full-resolution size, then tile-grid size.
    int4 Config; // Downsample factor, slots per tile, valid history, frame index.
};

[[vk::binding(20, 0)]] ConstantBuffer<RTGIGeometryConstants> Geometry;

struct RTGISurface
{
    float3 Position;
    float3 Normal;
    float3 GeometricNormal;
    float2 Pixel;
    float PixelSize;
};

bool RTGIValidTile(int2 Tile)
{
    return all(Tile >= 0) && all(Tile < Geometry.Sizes.zw);
}

RTGISurface RTGIGetSurface(float2 Pixel, float Depth, float2 EncodedNormal, bool Previous = false)
{
    RTGISurface Surface;
    const float2 Ndc = ScreenUVToNDC((Pixel + 0.5) / float2(Geometry.Sizes.xy));
    float4 Position;

    if (Previous)
    {
        Position = mul(Geometry.PreviousInverseViewProjection, float4(Ndc, Depth, 1));
    }
    else
    {
        Position = mul(Geometry.InverseViewProjection, float4(Ndc, Depth, 1));
    }

    Surface.Position        = Position.xyz / Position.w;
    Surface.Normal          = NormalDecode(EncodedNormal);
    Surface.GeometricNormal = Surface.Normal;
    Surface.Pixel           = Pixel;
    Surface.PixelSize       = max(0.0005, distance(Surface.Position, Geometry.CameraPositionPixelSize.xyz) * Geometry.CameraPositionPixelSize.w);
    return Surface;
}

// Reconstruct the previous position of the same surface, including object
// motion in depth. Existing previous-frame depth then validates visibility.
bool RTGIReprojectSurface(RTGISurface Surface, float Depth, float4 Motion, out RTGISurface Previous)
{
    Previous = Surface;

    if (!isfinite(Motion.z) || abs(Motion.z) >= MOTION_DEPTH_INVALID)
    {
        return false;
    }

    const float ViewZ = Geometry.DepthProjection.y / (Depth + Geometry.DepthProjection.x);
    const float PreviousViewZ = ViewZ + Motion.z;

    if (PreviousViewZ <= 0)
    {
        return false;
    }

    float PreviousDepth = Geometry.DepthProjection.w / PreviousViewZ - Geometry.DepthProjection.z;

    // Preserve raster precision when neither depth nor projection changed.
    if (Motion.z == 0 && all(Geometry.DepthProjection.xy == Geometry.DepthProjection.zw))
    {
        PreviousDepth = Depth;
    }

    if (IsSkyDepth(PreviousDepth) || PreviousDepth > 1)
    {
        return false;
    }

    const float2 PreviousPixel = Surface.Pixel - Motion.xy * 0.5 * Geometry.Sizes.xy + Geometry.JitterDelta.xy;
    Previous = RTGIGetSurface(PreviousPixel, PreviousDepth, float2(0.5, 0.5), true);
    Previous.Normal          = Surface.Normal;
    Previous.GeometricNormal = Surface.GeometricNormal;
    // FP16 stores relative precision in the delta, not the absolute depth.
    Previous.PixelSize = Surface.PixelSize + abs(Motion.z) * 0.0005;
    return true;
}

// XYZ stores world position; W losslessly packs the source pixel. Cached world
// positions avoid a matrix reconstruction for every spatial/upsampling tap.
// Two 15-bit coordinates cover 32768-pixel dimensions. Bias their packed bits
// into the normal float range so denormal flushing is harmless.
// The separate RGBA16F guide holds geometric normal XYZ and world pixel size W.
float RTGIPackPixel(int2 Pixel)
{
    return asfloat((uint(Pixel.x) | (uint(Pixel.y) << 15)) + 0x00800000u);
}

RTGISurface RTGIGetSample(float4 Source, float4 Normals)
{
    const uint Packed = asuint(Source.w) - 0x00800000u;
    RTGISurface Surface;
    Surface.Position        = Source.xyz;
    Surface.Pixel           = float2(Packed & 0x7fffu, Packed >> 15);
    Surface.Normal          = Normals.xyz;
    Surface.GeometricNormal = Normals.xyz;
    Surface.PixelSize       = Normals.w;
    return Surface;
}

// History and coverage use a strict two-sided plane test. Spatial reconstruction
// can follow local curvature using the average tangent plane. A bounded
// allowance avoids widening the filter across distant faces. Parallel depth
// layers retain their separation. Each caller weights screen-space distance.
float RTGISurfaceWeight(RTGISurface Center, RTGISurface Sample, bool AllowCurvature = false)
{
    const float NormalDot = dot(Center.GeometricNormal, Sample.GeometricNormal);

    if (NormalDot < 0.65)
    {
        return 0;
    }

    const float3 Delta = Sample.Position - Center.Position;

    const float CenterDistance = dot(Delta, Center.GeometricNormal);
    const float SampleDistance = dot(Delta, Sample.GeometricNormal);

    const float PixelSize  = max(Center.PixelSize, Sample.PixelSize);
    float       PlaneError = max(abs(CenterDistance), abs(SampleDistance));

    // Curvature gives opposite signed endpoint residuals. Same-sided offsets
    // describe a displaced surface and must keep the strict separation test.
    if (AllowCurvature && CenterDistance * SampleDistance < 0)
    {
        PlaneError = max(abs(CenterDistance + SampleDistance) * 0.5, PlaneError - 2 * PixelSize);
    }

    const float Tolerance = max(0.002, PixelSize * 0.75 + length(Delta) * 0.025);

    if (PlaneError >= Tolerance)
    {
        return 0;
    }

    const float NormalWeight = saturate((NormalDot - 0.65) / 0.35);
    const float PlaneWeight  = 1 - PlaneError / Tolerance;
    return NormalWeight * NormalWeight * PlaneWeight * PlaneWeight;
}

float3 RTGIFiniteRadiance(float3 Value)
{
    return all(isfinite(Value)) ? clamp(Value, 0, 65504) : 0;
}
