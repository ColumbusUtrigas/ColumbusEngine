#pragma once

#include "EnvBRDF.hlsli"

#ifndef REFLECTION_CAPTURES_SET
#error REFLECTION_CAPTURES_SET must be defined before including ReflectionCapturesCommon.hlsli
#endif

#ifndef REFLECTION_CAPTURE_MAX_CAPTURES
#define REFLECTION_CAPTURE_MAX_CAPTURES 32
#endif

struct ReflectionCaptureDesc
{
	float4 PositionIntensity;
	float4 ExtentBlendDistance; // xyz: full box size, w: blend distance.
	float4 PriorityMipEnabled;
};

struct ReflectionCaptureConstants
{
	int4 CountAndFlags;
	float4 Params;
	ReflectionCaptureDesc Captures[REFLECTION_CAPTURE_MAX_CAPTURES];
};

struct ReflectionCaptureSelection
{
	int Index0;
	int Index1;
	float Weight0;
	float Weight1;
};

[[vk::binding(0, REFLECTION_CAPTURES_SET)]] ConstantBuffer<ReflectionCaptureConstants> ReflectionCaptures;
[[vk::binding(1, REFLECTION_CAPTURES_SET)]] SamplerState ReflectionCaptureSampler;
[[vk::binding(2, REFLECTION_CAPTURES_SET)]] Texture2D<float2> ReflectionCaptureBRDFLUT;
[[vk::binding(3, REFLECTION_CAPTURES_SET)]] TextureCube<float4> ReflectionCaptureRadiance[REFLECTION_CAPTURE_MAX_CAPTURES];

bool ReflectionCapturesEnabled()
{
	return ReflectionCaptures.CountAndFlags.y != 0 && ReflectionCaptures.CountAndFlags.x > 0;
}

bool ReflectionCapturesRuntimeRTAvailable()
{
	return ReflectionCaptures.Params.z > 0.5;
}

float ReflectionCaptureInfluence(ReflectionCaptureDesc Capture, float3 Position)
{
	float3 HalfExtent = max(Capture.ExtentBlendDistance.xyz * 0.5, float3(0.001, 0.001, 0.001));
	float3 EdgeDistance = HalfExtent - abs(Position - Capture.PositionIntensity.xyz);

	if (any(EdgeDistance < 0.0))
	{
		return 0.0;
	}

	if (Capture.ExtentBlendDistance.w <= 0.0)
	{
		return 1.0;
	}

	float MinEdgeDistance = min(EdgeDistance.x, min(EdgeDistance.y, EdgeDistance.z));
	return saturate(MinEdgeDistance / max(Capture.ExtentBlendDistance.w, 0.001));
}

void ConsiderReflectionCapture(int Index, float Priority, float Weight, inout float BestPriority, inout ReflectionCaptureSelection Selection)
{
	if (Priority > BestPriority)
	{
		BestPriority = Priority;
		Selection.Index0 = Index;
		Selection.Index1 = -1;
		Selection.Weight0 = Weight;
		Selection.Weight1 = 0.0;
		return;
	}

	if (Priority != BestPriority)
	{
		return;
	}

	if (Weight > Selection.Weight0)
	{
		Selection.Index1 = Selection.Index0;
		Selection.Weight1 = Selection.Weight0;
		Selection.Index0 = Index;
		Selection.Weight0 = Weight;
	}
	else if (Weight > Selection.Weight1)
	{
		Selection.Index1 = Index;
		Selection.Weight1 = Weight;
	}
}

ReflectionCaptureSelection SelectReflectionCaptures(float3 Position)
{
	ReflectionCaptureSelection Selection;
	Selection.Index0 = -1;
	Selection.Index1 = -1;
	Selection.Weight0 = 0.0;
	Selection.Weight1 = 0.0;

	float BestPriority = -1.0;

	[loop]
	for (int i = 0; i < ReflectionCaptures.CountAndFlags.x; i++)
	{
		ReflectionCaptureDesc Capture = ReflectionCaptures.Captures[i];
		if (Capture.PriorityMipEnabled.z <= 0.0 || Capture.PositionIntensity.w <= 0.0)
			continue;

		float Weight = ReflectionCaptureInfluence(Capture, Position);
		if (Weight <= 0.0)
			continue;

		ConsiderReflectionCapture(i, Capture.PriorityMipEnabled.x, Weight, BestPriority, Selection);
	}

	float WeightSum = Selection.Weight0 + Selection.Weight1;
	if (WeightSum > 0.0)
	{
		Selection.Weight0 /= WeightSum;
		Selection.Weight1 /= WeightSum;
	}

	return Selection;
}

float3 ParallaxCorrectBoxDirection(ReflectionCaptureDesc Capture, float3 Position, float3 Direction)
{
	float3 HalfExtent = max(Capture.ExtentBlendDistance.xyz * 0.5, float3(0.001, 0.001, 0.001));
	float3 BoxMin = Capture.PositionIntensity.xyz - HalfExtent;
	float3 BoxMax = Capture.PositionIntensity.xyz + HalfExtent;

	float3 DirectionSign = float3(Direction.x >= 0.0 ? 1.0 : -1.0, Direction.y >= 0.0 ? 1.0 : -1.0, Direction.z >= 0.0 ? 1.0 : -1.0);
	float3 SafeDirection = DirectionSign * max(abs(Direction), float3(0.0001, 0.0001, 0.0001));
	float3 InvDirection = 1.0 / SafeDirection;
	float3 T0 = (BoxMin - Position) * InvDirection;
	float3 T1 = (BoxMax - Position) * InvDirection;
	float3 TFar = max(T0, T1);
	float HitT = min(TFar.x, min(TFar.y, TFar.z));

	if (HitT <= 0.0)
		return Direction;

	float3 HitPosition = Position + Direction * HitT;
	return normalize(HitPosition - Capture.PositionIntensity.xyz);
}

float3 SampleReflectionCaptureRadiance(int CaptureIndex, float3 Position, float3 ReflectionDirection, float Roughness)
{
	ReflectionCaptureDesc Capture = ReflectionCaptures.Captures[CaptureIndex];
	float3 CorrectedDirection = ParallaxCorrectBoxDirection(Capture, Position, ReflectionDirection);
	float Mip = EnvPrefilteredMip(Roughness, Capture.PriorityMipEnabled.y);
	return ReflectionCaptureRadiance[NonUniformResourceIndex(CaptureIndex)].SampleLevel(ReflectionCaptureSampler, CorrectedDirection, Mip).rgb * Capture.PositionIntensity.w;
}

float3 EvaluateReflectionCaptures(float3 Position, float3 Normal, float3 View, float3 F0, float Roughness, out bool HasCapture)
{
	HasCapture = false;
	if (!ReflectionCapturesEnabled())
	{
		return 0.0.xxx;
	}

	ReflectionCaptureSelection Selection = SelectReflectionCaptures(Position);
	if (Selection.Index0 < 0)
	{
		return 0.0.xxx;
	}

	HasCapture = true;
	float3 ReflectionDirection = reflect(-View, Normal);
	float3 PrefilteredRadiance = SampleReflectionCaptureRadiance(Selection.Index0, Position, ReflectionDirection, Roughness) * Selection.Weight0;
	if (Selection.Index1 >= 0)
	{
		PrefilteredRadiance += SampleReflectionCaptureRadiance(Selection.Index1, Position, ReflectionDirection, Roughness) * Selection.Weight1;
	}

	float NoV = saturate(dot(Normal, View));
	return ApplyEnvBRDF(ReflectionCaptureBRDFLUT, ReflectionCaptureSampler, PrefilteredRadiance, F0, Roughness, NoV);
}

float3 EvaluateReflectionCaptures(float3 Position, float3 Normal, float3 View, float3 F0, float Roughness)
{
	bool HasCapture = false;
	return EvaluateReflectionCaptures(Position, Normal, View, F0, Roughness, HasCapture);
}

float ReflectionCaptureRTBlend(float Roughness, bool RTReflectionsAvailable)
{
	if (!RTReflectionsAvailable)
	{
		return 1.0;
	}

	float MaxRTRoughness = saturate(ReflectionCaptures.Params.x);
	float FadeWidth = max(ReflectionCaptures.Params.y, 0.0001);
	return smoothstep(MaxRTRoughness - FadeWidth, MaxRTRoughness + FadeWidth, saturate(Roughness));
}

float3 ApplyReflectionCaptures(float3 RTReflection, float3 Position, float3 Normal, float3 View, float3 F0, float Roughness, bool RTReflectionsAvailable)
{
	float ProbeBlend = ReflectionCaptureRTBlend(Roughness, RTReflectionsAvailable);
	if (ProbeBlend <= 0.0)
	{
		return RTReflection;
	}

	bool HasCapture = false;
	float3 CaptureReflection = EvaluateReflectionCaptures(Position, Normal, View, F0, Roughness, HasCapture);
	if (!HasCapture)
	{
		return RTReflection;
	}

	return lerp(RTReflection, CaptureReflection, ProbeBlend);
}
