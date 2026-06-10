#pragma once

#include "Graphics/RenderGraph.h"

#include <vector>

namespace Columbus
{
	namespace RayTracingIrradianceVolumes
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
			Constants Data{};
			std::vector<RenderGraphBufferRef> ProbeBuffers;
		};

		inline Prepared Prepare(RenderGraph& Graph, RenderPassDependencies& Dependencies, bool bEnabled, VkPipelineStageFlags Stage)
		{
			Prepared Result{};
			Result.Data.CountAndFlags = iVector4(0, bEnabled ? 1 : 0, 0, 0);

			if (!bEnabled || Graph.Scene == nullptr)
			{
				return Result;
			}

			for (const IrradianceVolume& Volume : Graph.Scene->IrradianceVolumes)
			{
				if (Volume.ProbesBuffer == nullptr || Result.ProbeBuffers.size() >= MaxVolumes)
				{
					continue;
				}

				const int BufferIndex = (int)Result.ProbeBuffers.size();
				RenderGraphBufferRef ProbeBuffer = Graph.RegisterExternalBuffer(Volume.ProbesBuffer, "IrradianceProbes");
				Dependencies.ReadBuffer(ProbeBuffer, VK_ACCESS_SHADER_READ_BIT, Stage);
				Result.ProbeBuffers.push_back(ProbeBuffer);

				Result.Data.Volumes[BufferIndex] = VolumeDesc{
					.PositionIntensity = Vector4(Volume.Position, Volume.Intensity),
					.ExtentNormalBias = Vector4(Volume.Extent, Volume.NormalBias),
					.BlendPriority = Vector4(Volume.BlendDistance, Volume.Priority, 0.0f, 0.0f),
					.ProbesCountAndBufferIndex = iVector4(Volume.ProbesCount, BufferIndex),
				};
			}

			Result.Data.CountAndFlags.X = (int)Result.ProbeBuffers.size();
			if (Result.ProbeBuffers.empty())
			{
				Result.Data.CountAndFlags.Y = 0;
			}

			return Result;
		}

		inline void Bind(RenderGraphContext& Context, VkDescriptorSet DescriptorSet, const Prepared& PreparedData, Buffer* FallbackBuffer)
		{
			Constants Data = PreparedData.Data;
			Buffer* ConstantsBuffer = Context.Device->GetConstantBufferPrepared((u32)sizeof(Constants), &Data);
			Context.Device->UpdateDescriptorSet(DescriptorSet, 9, 0, ConstantsBuffer);

			for (int i = 0; i < MaxVolumes; i++)
			{
				Buffer* ProbeBuffer = i < (int)PreparedData.ProbeBuffers.size()
					? Context.GetRenderGraphBuffer(PreparedData.ProbeBuffers[i]).get()
					: FallbackBuffer;
				Context.Device->UpdateDescriptorSet(DescriptorSet, 10, i, ProbeBuffer);
			}
		}
	}
}
