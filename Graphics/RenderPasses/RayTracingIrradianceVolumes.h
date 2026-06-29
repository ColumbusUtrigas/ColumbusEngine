#pragma once

#include "Graphics/ShaderBinder.h"

#include <array>

namespace Columbus::RayTracingIrradianceVolumes
{

	static constexpr int MaxVolumes = 16;

	struct VolumeDesc
	{
		Vector4 PositionIntensity;
		Vector4 ExtentNormalBias;
		Vector4 BlendPriority;
		iVector4 ProbesCountAndBufferIndex;
	};

	struct Constants
	{
		iVector4 CountAndFlags;
		VolumeDesc Volumes[MaxVolumes]{};
	};

	struct Prepared
	{
		ShaderConstants<Constants> Constants;
		std::array<ShaderReadBuffer, MaxVolumes> ProbeBuffers;
	};

	inline Prepared Prepare(RenderGraph& Graph, RenderGraphBufferRef FallbackBuffer, bool bEnabled)
	{
		Prepared Result{};
		Result.Constants.Value.CountAndFlags = iVector4(0, bEnabled ? 1 : 0, 0, 0);

		for (int i = 0; i < MaxVolumes; i++)
		{
			Result.ProbeBuffers[i] = FallbackBuffer;
		}

		if (!bEnabled || Graph.Scene == nullptr)
		{
			return Result;
		}

		int VolumeIndex = 0;
		for (const IrradianceVolume& Volume : Graph.Scene->IrradianceVolumes)
		{
			if (Volume.ProbesBuffer == nullptr || VolumeIndex >= MaxVolumes)
			{
				continue;
			}

			RenderGraphBufferRef ProbeBuffer = Graph.RegisterExternalBuffer(Volume.ProbesBuffer, "IrradianceProbes");
			Result.ProbeBuffers[VolumeIndex] = ProbeBuffer;
			Result.Constants.Value.Volumes[VolumeIndex] = VolumeDesc{
				.PositionIntensity = Vector4(Volume.Position, Volume.Intensity),
				.ExtentNormalBias = Vector4(Volume.Extent, Volume.NormalBias),
				.BlendPriority = Vector4(Volume.BlendDistance, Volume.Priority, 0.0f, 0.0f),
				.ProbesCountAndBufferIndex = iVector4(Volume.ProbesCount, VolumeIndex),
			};

			VolumeIndex++;
		}

		Result.Constants.Value.CountAndFlags.X = VolumeIndex;
		if (VolumeIndex == 0)
		{
			Result.Constants.Value.CountAndFlags.Y = 0;
		}

		return Result;
	}

}
