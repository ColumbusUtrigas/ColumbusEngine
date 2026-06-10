#pragma once

#ifndef SET
#error SET must be defined before including RayTracingIrradianceVolumes.hlsli
#endif

#define IRRADIANCE_DECLARE_PROBE_STORAGE [[vk::binding(10, SET)]] StructuredBuffer<IrradianceProbe> IrradianceProbeBuffers[16];
#define IRRADIANCE_LOAD_PROBE(Index) IrradianceProbeBuffers[Volume.ProbeBufferIndex][Index]
#include "IrradianceVolume/IrradianceVolumeCommon.hlsli"

struct RuntimeIrradianceVolumeDesc
{
	float4 PositionIntensity;
	float4 ExtentNormalBias;
	float4 BlendPriority;
	int4 ProbesCountAndBufferIndex;
};

struct RuntimeIrradianceVolumeConstants
{
	int4 CountAndFlags; // x: count, y: enabled.
	RuntimeIrradianceVolumeDesc Volumes[16];
};

[[vk::binding(9, SET)]] ConstantBuffer<RuntimeIrradianceVolumeConstants> RuntimeIrradianceVolumes;

float3 SampleRuntimeIrradianceVolumes(float3 Position, float3 Normal)
{
	if (RuntimeIrradianceVolumes.CountAndFlags.y == 0)
	{
		return float3(0.0, 0.0, 0.0);
	}

	float3 Irradiance = float3(0.0, 0.0, 0.0);
	float AccumulatedWeight = 0.0;

	[loop]
	for (int i = 0; i < RuntimeIrradianceVolumes.CountAndFlags.x; i++)
	{
		RuntimeIrradianceVolumeDesc Desc = RuntimeIrradianceVolumes.Volumes[i];

		IrradianceVolume Volume;
		Volume.Position = Desc.PositionIntensity.xyz;
		Volume.Extent = Desc.ExtentNormalBias.xyz;
		Volume.ProbesCount = Desc.ProbesCountAndBufferIndex.xyz;
		Volume.ProbeBufferIndex = Desc.ProbesCountAndBufferIndex.w;
		Volume.NormalBias = Desc.ExtentNormalBias.w;
		Volume.BlendDistance = Desc.BlendPriority.x;
		Volume.Priority = Desc.BlendPriority.y;

		float Weight = IrradianceVolumeBlendWeight(Volume, Position);
		if (Weight > 0.0 && Desc.PositionIntensity.w > 0.0)
		{
			float SelectionWeight = Weight * IrradianceVolumePriorityWeight(Volume);
			Irradiance += SampleIrradianceProbes(Volume, Position, Normal) * SelectionWeight * Desc.PositionIntensity.w;
			AccumulatedWeight += SelectionWeight;
		}
	}

	return AccumulatedWeight > 0.0 ? Irradiance / AccumulatedWeight : float3(0.0, 0.0, 0.0);
}
