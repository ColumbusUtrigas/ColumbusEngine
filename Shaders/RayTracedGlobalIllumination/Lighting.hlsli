#pragma once

#include "../Common.hlsli"
#include "../SH.hlsli"

// Confidence controls reconstruction only. Spatially repaired lighting never
// increases the real temporal sample count or feeds back into probe history.
float RTGIProbeConfidence(float SampleCount)
{
    return lerp(0.125, 1.0, saturate((SampleCount - 1) / 15));
}

float RTGILuminance(float3 Colour)
{
    return dot(Colour, float3(0.2126, 0.7152, 0.0722));
}

// Full RGB radiance SH: 27 coefficients and mean ray distance in seven RGBA16F rows.
// Coefficients stay linear through temporal/spatial filtering. The cosine lobe
// is evaluated only at the receiver, independently of the probe's surface normal.
struct RTGIProbeSH
{
    float4 RedLow;
    float4 GreenLow;
    float4 BlueLow;
    float4 RedQuadratic;
    float4 GreenQuadratic;
    float4 BlueQuadratic;
    float4 Last;
};

RTGIProbeSH RTGIReadProbe(Texture2D<float4> Source, int2 Pixel, int Height)
{
    RTGIProbeSH Result;
    Result.RedLow         = Source[Pixel];
    Result.GreenLow       = Source[Pixel + int2(0, Height)];
    Result.BlueLow        = Source[Pixel + int2(0, Height * 2)];
    Result.RedQuadratic   = Source[Pixel + int2(0, Height * 3)];
    Result.GreenQuadratic = Source[Pixel + int2(0, Height * 4)];
    Result.BlueQuadratic  = Source[Pixel + int2(0, Height * 5)];
    Result.Last           = Source[Pixel + int2(0, Height * 6)];
    return Result;
}

void RTGIWriteProbe(RWTexture2D<float4> Target, int2 Pixel, int Height, RTGIProbeSH Value)
{
    Target[Pixel]                          = Value.RedLow;
    Target[Pixel + int2(0, Height)]         = Value.GreenLow;
    Target[Pixel + int2(0, Height * 2)]     = Value.BlueLow;
    Target[Pixel + int2(0, Height * 3)]     = Value.RedQuadratic;
    Target[Pixel + int2(0, Height * 4)]     = Value.GreenQuadratic;
    Target[Pixel + int2(0, Height * 5)]     = Value.BlueQuadratic;
    Target[Pixel + int2(0, Height * 6)]     = Value.Last;
}

void RTGIAddProbe(inout RTGIProbeSH Total, RTGIProbeSH Sample, float Weight)
{
    Total.RedLow         += Sample.RedLow * Weight;
    Total.GreenLow       += Sample.GreenLow * Weight;
    Total.BlueLow        += Sample.BlueLow * Weight;
    Total.RedQuadratic   += Sample.RedQuadratic * Weight;
    Total.GreenQuadratic += Sample.GreenQuadratic * Weight;
    Total.BlueQuadratic  += Sample.BlueQuadratic * Weight;
    Total.Last           += Sample.Last * Weight;
}

RTGIProbeSH RTGIScaleProbe(RTGIProbeSH Value, float Scale)
{
    RTGIProbeSH Result = (RTGIProbeSH)0;
    RTGIAddProbe(Result, Value, Scale);
    return Result;
}

RTGIProbeSH RTGIProjectProbe(float3 Radiance, float3 Direction, float HitDistance)
{
    const sh3 Basis = shEvaluate3(Direction);
    // One uniform-solid-angle hemisphere ray. The lower geometric hemisphere
    // has zero incident radiance; 1/PDF = 2*pi, with no cosine baked into SH.
    const float3 Weighted = Radiance * TWO_PI;
    RTGIProbeSH Result;
    Result.RedLow         = Basis.Low * Weighted.r;
    Result.GreenLow       = Basis.Low * Weighted.g;
    Result.BlueLow        = Basis.Low * Weighted.b;
    Result.RedQuadratic   = Basis.Quadratic * Weighted.r;
    Result.GreenQuadratic = Basis.Quadratic * Weighted.g;
    Result.BlueQuadratic  = Basis.Quadratic * Weighted.b;
    Result.Last           = float4(Basis.Last * Weighted, HitDistance);
    return Result;
}

float3 RTGIEvaluateProbe(RTGIProbeSH Probe, float3 Normal)
{
    const sh3 Transfer = shEvaluateDiffuse3(Normal);
    float3 Result;
    Result.r = dot(Probe.RedLow, Transfer.Low) + dot(Probe.RedQuadratic, Transfer.Quadratic) + Probe.Last.r * Transfer.Last;
    Result.g = dot(Probe.GreenLow, Transfer.Low) + dot(Probe.GreenQuadratic, Transfer.Quadratic) + Probe.Last.g * Transfer.Last;
    Result.b = dot(Probe.BlueLow, Transfer.Low) + dot(Probe.BlueQuadratic, Transfer.Quadratic) + Probe.Last.b * Transfer.Last;
    return max(Result, 0);
}

float3 RTGISampleHemisphere(float3 Normal, float2 Xi)
{
    const float Radius = sqrt(max(0, 1 - Xi.x * Xi.x));
    const float Angle  = TWO_PI * Xi.y;
    const float3 Local = float3(Radius * cos(Angle), Radius * sin(Angle), Xi.x);
    return mul(Local, ComputeTangentsFromVector(Normal));
}
