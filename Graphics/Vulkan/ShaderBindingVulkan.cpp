#include <Graphics/ShaderBinder.h>

#include <Graphics/RenderGraph.h>
#include <Graphics/Vulkan/CommandBufferVulkan.h>
#include <Graphics/Vulkan/DeviceVulkan.h>
#include <Graphics/Vulkan/ShaderBinderVulkan.h>

#include <cassert>
#include <cstdint>

namespace Columbus
{
	namespace
	{
		static ShaderBinderBackendStateVulkan& GetVulkanState(ShaderBinderBackendState* Backend)
		{
			assert(Backend != nullptr);
			return *static_cast<ShaderBinderBackendStateVulkan*>(Backend);
		}

		static void ResetVulkanState(ShaderBinderBackendState& Backend)
		{
			ShaderBinderBackendStateVulkan& VulkanState = static_cast<ShaderBinderBackendStateVulkan&>(Backend);
			for (VkDescriptorSet& Set : VulkanState.Sets)
			{
				Set = VK_NULL_HANDLE;
			}
			VulkanState.UsedMask = 0;
		}

		static VkDescriptorSet GetOrCreateDescriptorSet(
			ShaderBinderBackendState& Backend,
			RenderGraphContext& Context,
			const ComputePipeline* Compute,
			const GraphicsPipeline* Graphics,
			const RayTracingPipeline* RayTracing,
			int Set)
		{
			assert(Set >= 0 && Set < ShaderBinderBackendStateVulkan::MaxDescriptorSets);

			ShaderBinderBackendStateVulkan& VulkanState = static_cast<ShaderBinderBackendStateVulkan&>(Backend);
			const uint32_t SetBit = 1u << (uint32_t)Set;
			if ((VulkanState.UsedMask & SetBit) != 0)
			{
				return VulkanState.Sets[Set];
			}

			if (Compute != nullptr)
			{
				VulkanState.Sets[Set] = Context.GetDescriptorSet(Compute, Set);
			}
			else if (Graphics != nullptr)
			{
				VulkanState.Sets[Set] = Context.GetDescriptorSet(Graphics, Set);
			}
			else if (RayTracing != nullptr)
			{
				VulkanState.Sets[Set] = Context.GetDescriptorSet(RayTracing, Set);
			}
			else
			{
				assert(false);
			}

			VulkanState.UsedMask |= SetBit;
			return VulkanState.Sets[Set];
		}
	}

	ShaderBinder::ShaderBinder(RenderPassDependencies& InDependencies) :
		Dependencies(&InDependencies),
		CurrentMode(Mode::CollectDependencies)
	{
	}

	ShaderBinder::ShaderBinder(RenderGraphContext& InContext, const ComputePipeline* InPipeline, ShaderBinderBackendState& InBackend) :
		Context(&InContext),
		Compute(InPipeline),
		CurrentMode(Mode::BindCompute),
		Backend(&InBackend)
	{
		ResetVulkanState(InBackend);
	}

	ShaderBinder::ShaderBinder(RenderGraphContext& InContext, const GraphicsPipeline* InPipeline, ShaderBinderBackendState& InBackend) :
		Context(&InContext),
		Graphics(InPipeline),
		CurrentMode(Mode::BindGraphics),
		Backend(&InBackend)
	{
		ResetVulkanState(InBackend);
	}

	ShaderBinder::ShaderBinder(RenderGraphContext& InContext, const RayTracingPipeline* InPipeline, ShaderBinderBackendState& InBackend) :
		Context(&InContext),
		RayTracing(InPipeline),
		CurrentMode(Mode::BindRayTracing),
		Backend(&InBackend)
	{
		ResetVulkanState(InBackend);
	}

	bool ShaderBinder::IsCollectingDependencies() const
	{
		return CurrentMode == Mode::CollectDependencies;
	}

	bool ShaderBinder::IsBindingRuntime() const
	{
		return CurrentMode != Mode::CollectDependencies;
	}

	void ShaderBinder::Flush()
	{
		if (!IsBindingRuntime())
		{
			return;
		}

		ShaderBinderBackendStateVulkan& VulkanState = GetVulkanState(Backend);
		for (int Set = 0; Set < ShaderBinderBackendStateVulkan::MaxDescriptorSets; Set++)
		{
			const uint32_t SetBit = 1u << (uint32_t)Set;
			if ((VulkanState.UsedMask & SetBit) == 0)
			{
				continue;
			}

			switch (CurrentMode)
			{
			case Mode::BindCompute:
				Context->CommandBuffer->BindDescriptorSetsCompute(Compute, Set, 1, &VulkanState.Sets[Set]);
				break;
			case Mode::BindGraphics:
				Context->CommandBuffer->BindDescriptorSetsGraphics(Graphics, Set, 1, &VulkanState.Sets[Set]);
				break;
			case Mode::BindRayTracing:
				Context->CommandBuffer->BindDescriptorSetsRayTracing(RayTracing, Set, 1, &VulkanState.Sets[Set]);
				break;
			default:
				break;
			}
		}
	}

	void ShaderBinder::BindSampledTextureImpl(const ShaderSampledTexture& Resource, int Set, int Binding, int ArrayIndex)
	{
		if (IsCollectingDependencies())
		{
			if (Resource.Ref != -1)
			{
				Dependencies->Read(Resource.Ref, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
			}
			return;
		}

		SPtr<Texture2> GraphTexture;
		const Texture2* Texture = Resource.External;
		if (Texture == nullptr)
		{
			GraphTexture = Context->GetRenderGraphTexture(Resource.Ref);
			Texture = GraphTexture.get();
		}

		Context->Device->UpdateDescriptorSet(GetOrCreateDescriptorSet(*Backend, *Context, Compute, Graphics, RayTracing, Set), Binding, ArrayIndex, Texture, Resource.Aspect, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
	}

	void ShaderBinder::BindStorageTextureImpl(const ShaderStorageTexture& Resource, int Set, int Binding, int ArrayIndex)
	{
		if (IsCollectingDependencies())
		{
			if (Resource.Ref != -1)
			{
				Dependencies->Write(Resource.Ref, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
			}
			return;
		}

		SPtr<Texture2> GraphTexture;
		const Texture2* Texture = Resource.External;
		if (Texture == nullptr)
		{
			GraphTexture = Context->GetRenderGraphTexture(Resource.Ref);
			Texture = GraphTexture.get();
		}

		Context->Device->UpdateDescriptorSet(GetOrCreateDescriptorSet(*Backend, *Context, Compute, Graphics, RayTracing, Set), Binding, ArrayIndex, Texture, Resource.Aspect, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
	}

	void ShaderBinder::BindReadStorageTextureImpl(const ShaderReadStorageTexture& Resource, int Set, int Binding, int ArrayIndex)
	{
		if (IsCollectingDependencies())
		{
			if (Resource.Ref != -1)
			{
				Dependencies->Read(Resource.Ref, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
			}
			return;
		}

		SPtr<Texture2> GraphTexture;
		const Texture2* Texture = Resource.External;
		if (Texture == nullptr)
		{
			GraphTexture = Context->GetRenderGraphTexture(Resource.Ref);
			Texture = GraphTexture.get();
		}

		Context->Device->UpdateDescriptorSet(GetOrCreateDescriptorSet(*Backend, *Context, Compute, Graphics, RayTracing, Set), Binding, ArrayIndex, Texture, Resource.Aspect, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
	}

	void ShaderBinder::BindReadBufferImpl(const ShaderReadBuffer& Resource, int Set, int Binding, int ArrayIndex)
	{
		if (IsCollectingDependencies())
		{
			if (Resource.Ref != -1)
			{
				Dependencies->ReadBuffer(Resource.Ref, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
			}
			return;
		}

		SPtr<Buffer> GraphBuffer;
		const Buffer* Buffer = Resource.External;
		if (Buffer == nullptr)
		{
			GraphBuffer = Context->GetRenderGraphBuffer(Resource.Ref);
			Buffer = GraphBuffer.get();
		}

		Context->Device->UpdateDescriptorSet(GetOrCreateDescriptorSet(*Backend, *Context, Compute, Graphics, RayTracing, Set), Binding, ArrayIndex, Buffer);
	}

	void ShaderBinder::BindWriteBufferImpl(const ShaderWriteBuffer& Resource, int Set, int Binding, int ArrayIndex)
	{
		if (IsCollectingDependencies())
		{
			if (Resource.Ref != -1)
			{
				Dependencies->WriteBuffer(Resource.Ref, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
			}
			return;
		}

		SPtr<Buffer> GraphBuffer;
		const Buffer* Buffer = Resource.External;
		if (Buffer == nullptr)
		{
			GraphBuffer = Context->GetRenderGraphBuffer(Resource.Ref);
			Buffer = GraphBuffer.get();
		}

		Context->Device->UpdateDescriptorSet(GetOrCreateDescriptorSet(*Backend, *Context, Compute, Graphics, RayTracing, Set), Binding, ArrayIndex, Buffer);
	}

	void ShaderBinder::BindStaticSamplerImpl(const ShaderStaticSampler& Sampler, int Set, int Binding, int ArrayIndex)
	{
		if (IsBindingRuntime())
		{
			Context->Device->UpdateDescriptorSet(GetOrCreateDescriptorSet(*Backend, *Context, Compute, Graphics, RayTracing, Set), Binding, ArrayIndex, Context->Device->GetStaticSampler(Sampler.Desc));
		}
	}

	void ShaderBinder::BindSamplerImpl(const ShaderSampler& Sampler, int Set, int Binding, int ArrayIndex)
	{
		if (IsBindingRuntime())
		{
			Context->Device->UpdateDescriptorSet(GetOrCreateDescriptorSet(*Backend, *Context, Compute, Graphics, RayTracing, Set), Binding, ArrayIndex, Sampler.Value);
		}
	}

	void ShaderBinder::BindAccelerationStructureImpl(const ShaderAccelerationStructure& Resource, int Set, int Binding, int ArrayIndex)
	{
		if (IsBindingRuntime())
		{
			Context->Device->UpdateDescriptorSet(GetOrCreateDescriptorSet(*Backend, *Context, Compute, Graphics, RayTracing, Set), Binding, ArrayIndex, Resource.Value);
		}
	}

	void ShaderBinder::BindGPUSceneImpl(const ShaderGPUScene& Scene)
	{
		if (CurrentMode == Mode::BindGraphics)
		{
			Context->BindGPUScene(Graphics, Scene.UseCombinedSampler);
		}
		else if (CurrentMode == Mode::BindRayTracing)
		{
			Context->BindGPUScene(RayTracing, Scene.UseCombinedSampler);
		}
	}

	void ShaderBinder::BindDescriptorSetImpl(int Set)
	{
		if (IsBindingRuntime())
		{
			GetOrCreateDescriptorSet(*Backend, *Context, Compute, Graphics, RayTracing, Set);
		}
	}

	void ShaderBinder::BindConstantsData(const void* Data, size_t Size, int Set, int Binding, int ArrayIndex)
	{
		if (IsBindingRuntime())
		{
			Buffer* ConstantsBuffer = Context->Device->GetConstantBufferPrepared((u32)Size, const_cast<void*>(Data));
			Context->Device->UpdateDescriptorSet(GetOrCreateDescriptorSet(*Backend, *Context, Compute, Graphics, RayTracing, Set), Binding, ArrayIndex, ConstantsBuffer);
		}
	}

	void ShaderBinder::BindPushConstantsData(const void* Data, size_t Size, ShaderType Stages)
	{
		if (CurrentMode == Mode::BindCompute)
		{
			Context->CommandBuffer->PushConstantsCompute(Compute, Stages, 0, (uint32_t)Size, Data);
		}
		else if (CurrentMode == Mode::BindGraphics)
		{
			Context->CommandBuffer->PushConstantsGraphics(Graphics, Stages, 0, (uint32_t)Size, Data);
		}
		else if (CurrentMode == Mode::BindRayTracing)
		{
			Context->CommandBuffer->PushConstantsRayTracing(RayTracing, Stages, 0, (uint32_t)Size, Data);
		}
	}
}
