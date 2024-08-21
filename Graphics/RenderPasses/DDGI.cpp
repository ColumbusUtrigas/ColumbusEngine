#include "Graphics/Core/Pipelines.h"
#include "Graphics/RenderGraph.h"
#include "RenderPasses.h"

#include "DDGI/Defines.h" // common defines, both for SDK and shaders, TODO: actually unify

// RTXGI SDK headers
#include "DDGI/SDK/include/rtxgi/ddgi/DDGIVolume.h"
#include "DDGI/SDK/include/rtxgi/ddgi/gfx/DDGIVolume_VK.h"
#include "DDGI/SDK/include/rtxgi/VulkanExtensions.h"

#include "Graphics/Vulkan/BufferVulkan.h"
#include "ShaderBytecode/ShaderBytecode.h"

namespace Columbus
{

struct DDGIApplyParameters
{
	Vector4  CameraPos;
	iVector2 Resolution;
};

void RenderIndirectLightingDDGI(RenderGraph& Graph, const RenderView& View, SceneTextures& Textures, DeferredRenderContext& DeferredContext)
{
	// TODO: DDGI doesn't work now, because DXC for SPIRV doesn't
	// support RayTracing without RayQuery extensions
	// it should be fixed in the next release of DXC, but for now, it doesn't work
	//return;

	// TODO: define lifetime, move to a more appropriate place
	static rtxgi::vulkan::DDGIVolume* sdkVolume = nullptr;
	static Buffer* VolumeConstants = nullptr;
	static Buffer* UploadBuffers[MaxFramesInFlight] {nullptr};
	static int FrameIndex = 0; // for buffer upload

	// MaxFramesInFlight
	// define max number of volumes at the same time

	// PLAN:
	// + 1. understand how to upload constants and implement it
	// + 2. trace rays and start ray dispatch
	// + 3. run everything else and validate that it works
	// 4. apply results on top of GBuffer into a lighting texture to be used later

	{
		iVector2 RenderSize = View.RenderSize;
		Vector3 CameraPos = View.CameraCur.Pos;

		TextureDesc2 Desc{
			.Usage = TextureUsage::Storage | TextureUsage::Sampled,
			.Width = (u32)RenderSize.X,
			.Height = (u32)RenderSize.Y,
			.Format = TextureFormat::RGBA16F,
		};

		RenderGraphTextureRef GI_Tex = Graph.CreateTexture(Desc, "DDGI_Output");
		Textures.RTGI = GI_Tex;

		RenderPassParameters Parameters;
		RenderPassDependencies Dependencies(Graph.Allocator);
		Dependencies.Read(Textures.GBufferWP, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
		Dependencies.Read(Textures.GBufferNormal, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
		Dependencies.Write(GI_Tex, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

		Graph.AddPass("DDGI", RenderGraphPassType::Compute, Parameters, Dependencies, [Textures, RenderSize, CameraPos, GI_Tex](RenderGraphContext& Context)
		{
			// TODO: separate volume creation
			if (sdkVolume == nullptr)
			{
				rtxgi::vulkan::LoadExtensions(Context.Device->_Device);

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
				Desc.origin = { 0, 4, 0 };
				Desc.probeCounts = { 8, 8, 8 };
				Desc.probeSpacing = { 1, 1, 1 };
				Desc.probeRayDataFormat     = rtxgi::EDDGIVolumeTextureFormat::F32x4;
				Desc.probeIrradianceFormat  = rtxgi::EDDGIVolumeTextureFormat::F16x4;
				Desc.probeDistanceFormat    = rtxgi::EDDGIVolumeTextureFormat::F16x2; // f16 might not be enough
				Desc.probeDataFormat        = rtxgi::EDDGIVolumeTextureFormat::F16x4;
				Desc.probeVariabilityFormat = rtxgi::EDDGIVolumeTextureFormat::F16;
				Desc.probeNumIrradianceTexels = NumTexels;
				Desc.probeNumIrradianceInteriorTexels = NumInteriorTexels;
				Desc.probeNumDistanceTexels = NumTexels;
				Desc.probeNumDistanceInteriorTexels = NumInteriorTexels;
				Desc.probeRelocationEnabled = true;
				Desc.probeClassificationEnabled = false;

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

			sdkVolume->Update();

			// volume constant buffer upload
			rtxgi::DDGIVolumeDescGPUPacked Packed = sdkVolume->GetDescGPUPacked();
			void* UploadMemory = Context.Device->MapBuffer(CurrentUploadBuffer);
			memcpy(UploadMemory, &Packed, sizeof(Packed));
			Context.Device->UnmapBuffer(CurrentUploadBuffer);
			Context.CommandBuffer->CopyBuffer(CurrentUploadBuffer, VolumeConstants, 0, 0, sizeof(Packed));

			// sync
			// TODO: RHI sync
			{
				BufferVulkan* VolumeVkBuffer = static_cast<BufferVulkan*>(VolumeConstants);

				VkBufferMemoryBarrier VkBarrier;
				VkBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
				VkBarrier.pNext = nullptr;
				VkBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				VkBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				VkBarrier.srcQueueFamilyIndex = 0;
				VkBarrier.dstQueueFamilyIndex = 0;
				VkBarrier.buffer = VolumeVkBuffer->_Buffer;
				VkBarrier.offset = 0;
				VkBarrier.size = VolumeVkBuffer->GetSize();

				vkCmdPipelineBarrier(Context.CommandBuffer->_CmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, 0,
					0, 0, // memory barrier
					1, &VkBarrier, // buffer memory barrier
					0, 0  // image memory barrier
				);
			}

			TextureVulkan ProbeData({});
			ProbeData._Layout  = VK_IMAGE_LAYOUT_GENERAL;
			ProbeData._Image   = sdkVolume->GetProbeData();
			ProbeData._View    = sdkVolume->GetProbeDataView();
			ProbeData._Sampler = NULL;

			TextureVulkan RayData({});
			RayData._Layout  = VK_IMAGE_LAYOUT_GENERAL;
			RayData._Image   = sdkVolume->GetProbeRayData();
			RayData._View    = sdkVolume->GetProbeRayDataView();
			RayData._Sampler = NULL;

			TextureVulkan ProbeIrradiance({});
			ProbeIrradiance._Layout  = VK_IMAGE_LAYOUT_GENERAL;
			ProbeIrradiance._Image   = sdkVolume->GetProbeIrradiance();
			ProbeIrradiance._View    = sdkVolume->GetProbeIrradianceView();
			ProbeIrradiance._Sampler = NULL;

			TextureVulkan ProbeDistance({});
			ProbeDistance._Layout  = VK_IMAGE_LAYOUT_GENERAL;
			ProbeDistance._Image   = sdkVolume->GetProbeDistance();
			ProbeDistance._View    = sdkVolume->GetProbeDistanceView();
			ProbeDistance._Sampler = NULL;

			{
				Context.CommandBuffer->TransitionImageLayout(&ProbeData, VK_IMAGE_LAYOUT_GENERAL);
				Context.CommandBuffer->TransitionImageLayout(&RayData, VK_IMAGE_LAYOUT_GENERAL);
				Context.CommandBuffer->TransitionImageLayout(&ProbeIrradiance, VK_IMAGE_LAYOUT_GENERAL);
				Context.CommandBuffer->TransitionImageLayout(&ProbeDistance, VK_IMAGE_LAYOUT_GENERAL);
			}

			{
				Context.CommandBuffer->BeginDebugMarker("DDGI Trace Rays");

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

				auto TlasSet = Context.GetDescriptorSet(TracePipeline, 2);
				Context.Device->UpdateDescriptorSet(TlasSet, 0, 0, Context.Scene->TLAS);
				Context.Device->UpdateDescriptorSet(TlasSet, 1, 0, VolumeConstants);
				Context.Device->UpdateDescriptorSet(TlasSet, 2, 0, &ProbeData,       TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(TlasSet, 3, 0, &RayData,         TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
				Context.Device->UpdateDescriptorSet(TlasSet, 4, 0, &ProbeIrradiance, TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(TlasSet, 5, 0, &ProbeDistance,   TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(TlasSet, 6, 0, Context.Device->GetStaticSampler());

				Context.CommandBuffer->BindRayTracingPipeline(TracePipeline);
				Context.BindGPUScene(TracePipeline, false);
				Context.CommandBuffer->BindDescriptorSetsRayTracing(TracePipeline, 2, 1, &TlasSet);
				Context.CommandBuffer->TraceRays(TracePipeline, Width, Height, Depth);
				Context.CommandBuffer->EndDebugMarker();
			}

			{
				Context.CommandBuffer->TransitionImageLayout(&ProbeData, VK_IMAGE_LAYOUT_GENERAL);
				Context.CommandBuffer->TransitionImageLayout(&RayData, VK_IMAGE_LAYOUT_GENERAL);
				Context.CommandBuffer->TransitionImageLayout(&ProbeIrradiance, VK_IMAGE_LAYOUT_GENERAL);
				Context.CommandBuffer->TransitionImageLayout(&ProbeDistance, VK_IMAGE_LAYOUT_GENERAL);
			}

			rtxgi::vulkan::UpdateDDGIVolumeProbes(Context.CommandBuffer->_CmdBuf, 1, &sdkVolume);
			rtxgi::vulkan::RelocateDDGIVolumeProbes(Context.CommandBuffer->_CmdBuf, 1, &sdkVolume);
			rtxgi::vulkan::ClassifyDDGIVolumeProbes(Context.CommandBuffer->_CmdBuf, 1, &sdkVolume);
			rtxgi::vulkan::CalculateDDGIVolumeVariability(Context.CommandBuffer->_CmdBuf, 1, &sdkVolume);
			rtxgi::vulkan::ReadbackDDGIVolumeVariability(Context.Device->_Device, 1, &sdkVolume);

			{
				Context.CommandBuffer->BeginDebugMarker("DDGI Apply");

				static ComputePipeline* Pipeline = nullptr;
				if (Pipeline == nullptr)
				{
					ComputePipelineDesc Desc;
					Desc.Name = "DDGI Trace";
					Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/DDGI/Apply.csd");
					Pipeline = Context.Device->CreateComputePipeline(Desc);
				}

				auto Set = Context.GetDescriptorSet(Pipeline, 0);
				Context.Device->UpdateDescriptorSet(Set, 0, 0, Context.GetRenderGraphTexture(Textures.GBufferWP).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 1, 0, Context.GetRenderGraphTexture(Textures.GBufferNormal).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 2, 0, VolumeConstants);
				Context.Device->UpdateDescriptorSet(Set, 3, 0, &ProbeData, TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 4, 0, &ProbeIrradiance, TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 5, 0, &ProbeDistance, TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(Set, 6, 0, Context.Device->GetStaticSampler());
				Context.Device->UpdateDescriptorSet(Set, 7, 0, Context.GetRenderGraphTexture(GI_Tex).get());

				DDGIApplyParameters Params{
					.CameraPos = Vector4(CameraPos, 0),
					.Resolution = RenderSize,
				};

				Context.CommandBuffer->BindComputePipeline(Pipeline);
				Context.CommandBuffer->BindDescriptorSetsCompute(Pipeline, 0, 1, &Set);
				Context.CommandBuffer->PushConstantsCompute(Pipeline, ShaderType::Compute, 0, sizeof(Params), &Params);
				Context.DispatchComputePixels(Pipeline, { 8, 8, 1 }, { RenderSize , 1});

				Context.CommandBuffer->EndDebugMarker();
			}
		});
	}
}

}
