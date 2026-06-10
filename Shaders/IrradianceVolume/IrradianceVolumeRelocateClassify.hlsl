#define IRRADIANCE_PROBES_SET 0
#include "IrradianceVolumeCommon.hlsli"

[[vk::binding(1, 0)]] RWStructuredBuffer<IrradianceFixedRayData> IrradianceFixedRayDataBuffer;

[[vk::push_constant]]
struct _Params
{
	float4 Position;
	float4 Extent;
	int4 ProbesCount;
	int TotalProbes;
	int FixedRayCount;
	int Iteration;
	int Mode; // 0 relocation/classification, 1 DDGI-style visibility moment blend.
	int RayDataStride;
	int VisibilityRayCount;
} Params;

float3 ProbeSpacing()
{
	return Params.Extent.xyz / max((float3)Params.ProbesCount.xyz, float3(1, 1, 1));
}

int3 ProbeCoordsFromIndex(int ProbeIndex)
{
	return int3(
		ProbeIndex / (Params.ProbesCount.y * Params.ProbesCount.z),
		(ProbeIndex / Params.ProbesCount.z) % Params.ProbesCount.y,
		ProbeIndex % Params.ProbesCount.z);
}

float3 BaseProbePosition(int ProbeIndex)
{
	float3 CellSize = ProbeSpacing();
	return Params.Position.xyz - Params.Extent.xyz * 0.5 + ((float3)ProbeCoordsFromIndex(ProbeIndex) + 0.5) * CellSize;
}

float RayDistance(int ProbeIndex, int RayIndex)
{
	return IrradianceFixedRayDataBuffer[ProbeIndex * Params.RayDataStride + RayIndex].Distance;
}

[numthreads(64, 1, 1)]
void main(uint3 DispatchThreadID : SV_DispatchThreadID)
{
	if (Params.Mode == 1)
	{
		int ProbeTexelIndex = DispatchThreadID.x;
		int TotalTexels = Params.TotalProbes * 64;
		if (ProbeTexelIndex >= TotalTexels)
		{
			return;
		}

		int ProbeIndex = ProbeTexelIndex / 64;
		int DistanceTexelIndex = ProbeTexelIndex % 64;

		IrradianceProbe Probe = IrradianceProbes[ProbeIndex];
		if (Probe.PositionState.w < 0.5)
		{
			Probe.Distance[DistanceTexelIndex] = float2(0, 0);
			IrradianceProbes[ProbeIndex] = Probe;
			return;
		}

		float3 TexelDirection = IrradianceDistanceTexelDirection(DistanceTexelIndex);
		float ProbeMaxRayDistance = length(ProbeSpacing()) * 1.5;
		float4 Result = float4(0, 0, 0, 0);

		for (int RayIndex = 0; RayIndex < Params.VisibilityRayCount; RayIndex++)
		{
			float3 RayDirection = IrradianceSphericalFibonacci(RayIndex, Params.VisibilityRayCount);
			float Weight = max(0.0, dot(TexelDirection, RayDirection));
			if (Weight <= 0.0)
			{
				continue;
			}

			Weight = pow(Weight, 50.0);
			float ProbeRayDistance = min(abs(RayDistance(ProbeIndex, RayIndex)), ProbeMaxRayDistance);
			Result += float4(ProbeRayDistance * Weight, ProbeRayDistance * ProbeRayDistance * Weight, 0, Weight);
		}

		float Epsilon = max((float)Params.VisibilityRayCount, 1.0) * 1e-9;
		Result.rg *= 1.0 / (2.0 * max(Result.a, Epsilon));
		Probe.Distance[DistanceTexelIndex] = Result.rg;
		IrradianceProbes[ProbeIndex] = Probe;
		return;
	}

	int ProbeIndex = DispatchThreadID.x;
	if (ProbeIndex >= Params.TotalProbes)
	{
		return;
	}

	IrradianceProbe Probe = IrradianceProbes[ProbeIndex];
	float3 BasePosition = BaseProbePosition(ProbeIndex);
	float3 CurrentOffset = Probe.PositionState.xyz - BasePosition;
	float3 Spacing = ProbeSpacing();
	float MinSpacing = min(Spacing.x, min(Spacing.y, Spacing.z));
	float MinFrontfaceDistance = 0.25 * MinSpacing;

	int NumFixedRays = min(Params.FixedRayCount, 32);
	int BackfaceCount = 0;
	int ClosestBackfaceIndex = -1;
	int ClosestFrontfaceIndex = -1;
	int FarthestFrontfaceIndex = -1;
	float ClosestBackfaceDistance = 1e27;
	float ClosestFrontfaceDistance = 1e27;
	float FarthestFrontfaceDistance = 0.0;

	for (int RayIndex = 0; RayIndex < NumFixedRays; RayIndex++)
	{
		float HitDistance = RayDistance(ProbeIndex, RayIndex);
		if (HitDistance < 0.0)
		{
			BackfaceCount++;
			HitDistance = HitDistance * -5.0;
			if (HitDistance < ClosestBackfaceDistance)
			{
				ClosestBackfaceDistance = HitDistance;
				ClosestBackfaceIndex = RayIndex;
			}
		}
		else
		{
			if (HitDistance < ClosestFrontfaceDistance)
			{
				ClosestFrontfaceDistance = HitDistance;
				ClosestFrontfaceIndex = RayIndex;
			}
			if (HitDistance > FarthestFrontfaceDistance && HitDistance < 1e20)
			{
				FarthestFrontfaceDistance = HitDistance;
				FarthestFrontfaceIndex = RayIndex;
			}
		}
	}

	float BackfaceRatio = NumFixedRays > 0 ? (float)BackfaceCount / NumFixedRays : 0.0;
	float3 FullOffset = float3(1e27, 1e27, 1e27);

	if (ClosestBackfaceIndex != -1 && BackfaceRatio > 0.25)
	{
		float3 Direction = IrradianceFixedRayDirection(ClosestBackfaceIndex);
		FullOffset = CurrentOffset + Direction * (ClosestBackfaceDistance + MinFrontfaceDistance * 0.5);
	}
	else if (ClosestFrontfaceDistance < MinFrontfaceDistance && FarthestFrontfaceIndex != -1)
	{
		float3 ClosestDirection = IrradianceFixedRayDirection(ClosestFrontfaceIndex);
		float3 FarthestDirection = IrradianceFixedRayDirection(FarthestFrontfaceIndex);
		if (dot(ClosestDirection, FarthestDirection) <= 0.0)
		{
			FullOffset = CurrentOffset + FarthestDirection * min(FarthestFrontfaceDistance, 1.0);
		}
	}
	else if (ClosestFrontfaceDistance > MinFrontfaceDistance && length(CurrentOffset) > 0.0001)
	{
		float MoveBack = min(ClosestFrontfaceDistance - MinFrontfaceDistance, length(CurrentOffset));
		FullOffset = CurrentOffset - normalize(CurrentOffset) * MoveBack;
	}

	float3 NormalizedOffset = FullOffset / max(Spacing, float3(0.001, 0.001, 0.001));
	if (dot(NormalizedOffset, NormalizedOffset) < 0.2025)
	{
		CurrentOffset = FullOffset;
	}

	bool Active = BackfaceRatio <= 0.25;

	Probe.PositionState = float4(BasePosition + CurrentOffset, Active ? 1.0 : 0.0);
	IrradianceProbes[ProbeIndex] = Probe;
}
