#include "Graphics/Core/Pipelines.h"
#include "Graphics/RenderGraph.h"
#include "RenderPasses.h"

#include "DDGI/Defines.h" // common defines, both for SDK and shaders, TODO: actually unify

// RTXGI SDK headers
#include "DDGI/SDK/include/rtxgi/ddgi/DDGIVolume.h"
#include "DDGI/SDK/include/rtxgi/ddgi/gfx/DDGIVolume_VK.h"

#include "Graphics/Vulkan/BufferVulkan.h"
#include "ShaderBytecode/ShaderBytecode.h"

namespace Columbus
{

void RenderIndirectLightingDDGI(RenderGraph& Graph, const RenderView& View)
{
	// TODO: DDGI doesn't work now, because DXC for SPIRV doesn't
	// support RayTracing without RayQuery extensions
	// it should be fixed in the next release of DXC, but for now, it doesn't work
	return;

	// TODO: define lifetime, move to a more appropriate place
	static rtxgi::vulkan::DDGIVolume* sdkVolume = nullptr;
	static Buffer* VolumeConstants = nullptr;
	static Buffer* UploadBuffers[MaxFramesInFlight] {nullptr};
	static int FrameIndex = 0; // for buffer upload

	// MaxFramesInFlight
	// define max number of volumes at the same time

	// PLAN:
	// 1. understand how to upload constants and implement it
	// 2. trace rays and start ray dispatch
	// 3. run everything else and validate that it works
	// 4. apply results on top of GBuffer into a lighting texture to be used later

	{
		RenderPassParameters Parameters;
		RenderPassDependencies Dependencies;
		
		Graph.AddPass("DDGI", RenderGraphPassType::Compute, Parameters, Dependencies, [](RenderGraphContext& Context)
		{
			if (sdkVolume == nullptr)
			{
				{
					rtxgi::DDGIVolumeDescGPUPacked Packed;
					BufferDesc ConstantsDesc(sizeof(Packed), BufferType::UAV);
					VolumeConstants = Context.Device->CreateBuffer(ConstantsDesc, nullptr);

					for (int i = 0; i < MaxFramesInFlight; i++)
					{
						BufferDesc UploadDesc(sizeof(Packed), BufferType::UAV);
						UploadDesc.HostVisible = true;
						UploadBuffers[i] = Context.Device->CreateBuffer(UploadDesc, nullptr);
					}
				}

				sdkVolume = new rtxgi::vulkan::DDGIVolume();

				// must match shader Defines.h
				const int NumTexels = 8;
				const int NumInteriorTexels = 6;

				rtxgi::DDGIVolumeDesc Desc;
				Desc.probeCounts = { 8, 8, 8 };
				Desc.probeSpacing = { 100, 100, 100 };
				Desc.probeRayDataFormat     = rtxgi::EDDGIVolumeTextureFormat::F32x2;
				Desc.probeIrradianceFormat  = rtxgi::EDDGIVolumeTextureFormat::F16x4;
				Desc.probeDistanceFormat    = rtxgi::EDDGIVolumeTextureFormat::F16x2; // f16 might not be enough
				Desc.probeDataFormat        = rtxgi::EDDGIVolumeTextureFormat::F16x4;
				Desc.probeVariabilityFormat = rtxgi::EDDGIVolumeTextureFormat::F16;
				Desc.probeNumIrradianceTexels = NumTexels;
				Desc.probeNumIrradianceInteriorTexels = NumInteriorTexels;
				Desc.probeNumDistanceTexels = NumTexels;
				Desc.probeNumDistanceInteriorTexels = NumInteriorTexels;

				CompiledShaderData BlendDistance = LoadCompiledShaderData("./PrecompiledShaders/DDGI/ProbeBlendingDistance.csd");
				CompiledShaderData BlendRadiance = LoadCompiledShaderData("./PrecompiledShaders/DDGI/ProbeBlendingRadiance.csd");

				CompiledShaderData ProbeRelocationUpdate = LoadCompiledShaderData("./PrecompiledShaders/DDGI/ProbeRelocation.csd");
				CompiledShaderData ProbeRelocationReset  = LoadCompiledShaderData("./PrecompiledShaders/DDGI/ProbeRelocationReset.csd");

				CompiledShaderData ProbeClassificationUpdate = LoadCompiledShaderData("./PrecompiledShaders/DDGI/ProbeClassification.csd");
				CompiledShaderData ProbeClassificationReset  = LoadCompiledShaderData("./PrecompiledShaders/DDGI/ProbeClassificationReset.csd");

				CompiledShaderData ProbeVariabilityUpdate = LoadCompiledShaderData("./PrecompiledShaders/DDGI/ProbeVariabilityReduction.csd");
				CompiledShaderData ProbeVariabilityExtra  = LoadCompiledShaderData("./PrecompiledShaders/DDGI/ProbeVariabilityExtraReduction.csd");

				rtxgi::vulkan::DDGIVolumeResources Resources;
				Resources.bindless.enabled = false;
				Resources.unmanaged.enabled = false;
				Resources.managed.enabled = true;
				Resources.constantsBuffer = static_cast<BufferVulkan*>(VolumeConstants)->_Buffer;
				Resources.managed.device = Context.Device->_Device;
				Resources.managed.physicalDevice = Context.Device->_PhysicalDevice;
				Resources.managed.descriptorPool = Context.Device->_DescriptorPool;

				Resources.managed.probeBlendingDistanceCS.pData = BlendDistance.Bytecodes[0].Bytecode.data();
				Resources.managed.probeBlendingDistanceCS.size  = BlendDistance.Bytecodes[0].Bytecode.size();
				Resources.managed.probeBlendingIrradianceCS.pData = BlendRadiance.Bytecodes[0].Bytecode.data();
				Resources.managed.probeBlendingIrradianceCS.size  = BlendRadiance.Bytecodes[0].Bytecode.size();

				Resources.managed.probeRelocation.updateCS.pData = ProbeRelocationUpdate.Bytecodes[0].Bytecode.data();
				Resources.managed.probeRelocation.updateCS.size  = ProbeRelocationUpdate.Bytecodes[0].Bytecode.size();
				Resources.managed.probeRelocation.resetCS.pData  = ProbeRelocationReset.Bytecodes[0].Bytecode.data();
				Resources.managed.probeRelocation.resetCS.size   = ProbeRelocationReset.Bytecodes[0].Bytecode.size();

				Resources.managed.probeClassification.updateCS.pData = ProbeClassificationUpdate.Bytecodes[0].Bytecode.data();
				Resources.managed.probeClassification.updateCS.size  = ProbeClassificationUpdate.Bytecodes[0].Bytecode.size();
				Resources.managed.probeClassification.resetCS.pData  = ProbeClassificationReset.Bytecodes[0].Bytecode.data();
				Resources.managed.probeClassification.resetCS.size   = ProbeClassificationReset.Bytecodes[0].Bytecode.size();

				Resources.managed.probeVariability.reductionCS.pData      = ProbeVariabilityUpdate.Bytecodes[0].Bytecode.data();
				Resources.managed.probeVariability.reductionCS.size       = ProbeVariabilityUpdate.Bytecodes[0].Bytecode.size();
				Resources.managed.probeVariability.extraReductionCS.pData = ProbeVariabilityExtra.Bytecodes[0].Bytecode.data();
				Resources.managed.probeVariability.extraReductionCS.size  = ProbeVariabilityExtra.Bytecodes[0].Bytecode.size();

				rtxgi::ERTXGIStatus Status = sdkVolume->Create(Context.CommandBuffer->_CmdBuf, Desc, Resources);
				COLUMBUS_ASSERT(Status == rtxgi::ERTXGIStatus::OK);
			}

			FrameIndex = FrameIndex++ % MaxFramesInFlight;
			Buffer* CurrentUploadBuffer = UploadBuffers[FrameIndex];

			// volume constant buffer upload
			rtxgi::DDGIVolumeDescGPUPacked Packed = sdkVolume->GetDescGPUPacked();
			void* UploadMemory = Context.Device->MapBuffer(CurrentUploadBuffer);
			memcpy(UploadMemory, &Packed, sizeof(Packed));
			Context.Device->UnmapBuffer(CurrentUploadBuffer);
			Context.CommandBuffer->CopyBuffer(CurrentUploadBuffer, VolumeConstants, 0, 0, sizeof(Packed));
			// TODO: sync

			{
				static RayTracingPipeline* TracePipeline = nullptr;
				if (TracePipeline == nullptr)
				{
					RayTracingPipelineDesc Desc;
					Desc.Name = "DDGI Trace";
					Desc.MaxRecursionDepth = 1;
					Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/DDGI/ProbeUpdate.csd");
					TracePipeline = Context.Device->CreateRayTracingPipeline(Desc);
				}

				u32 Width, Height, Depth;
				sdkVolume->GetRayDispatchDimensions(Width, Height, Depth);

				Context.BindGPUScene(TracePipeline);

				auto TlasSet = Context.GetDescriptorSet(TracePipeline, 7);
				Context.Device->UpdateDescriptorSet(TlasSet, 0, 0, Context.Scene->TLAS);

				Context.CommandBuffer->BeginDebugMarker("DDGI Trace Rays");
				Context.CommandBuffer->BindRayTracingPipeline(TracePipeline);
				Context.CommandBuffer->EndDebugMarker();
			}

			// rtxgi::vulkan::UploadDDGIVolumeConstants(VkDevice device, VkCommandBuffer cmdBuffer, uint32_t bufferingIndex, uint32_t numVolumes, DDGIVolume **volumes)
			// TODO: trace rays, GetRayDispatchDimensions()
			// rtxgi::vulkan::UpdateDDGIVolumeProbes(Context.CommandBuffer->_CmdBuf, 1, &sdkVolume);
			// rtxgi::vulkan::RelocateDDGIVolumeProbes(Context.CommandBuffer->_CmdBuf, 1, &sdkVolume);
			// rtxgi::vulkan::ClassifyDDGIVolumeProbes(Context.CommandBuffer->_CmdBuf, 1, &sdkVolume);
			// rtxgi::vulkan::CalculateDDGIVolumeVariability(Context.CommandBuffer->_CmdBuf, 1, &sdkVolume);
		});
	}
}

}
