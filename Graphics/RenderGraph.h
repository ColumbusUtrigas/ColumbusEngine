#pragma once

#include <Core/Core.h>
#include <Graphics/Vulkan/DeviceVulkan.h>
#include "Core/fixed_vector.h"
#include "GPUScene.h"
#include "Graphics/RayTracingPipeline.h"
#include "Graphics/Types.h"
#include "Graphics/Vulkan/CommandBufferVulkan.h"
#include "Graphics/Vulkan/FenceVulkan.h"
#include "Graphics/Vulkan/SwapchainVulkan.h"
#include <memory>
#include <vector>
#include <unordered_map>

#include <vulkan/vulkan.h>

namespace Columbus
{

	class RenderGraph;

	using RenderGraphResourceId = int;

	constexpr int MaxFramesInFlight = 2;

	struct RenderGraphData
	{
		std::unordered_map<std::string, Buffer*> Buffers; // TODO
		std::unordered_map<std::string, Texture2*> Textures; // TODO

		struct
		{
			SPtr<FenceVulkan> Fence;
			VkSemaphore ImageSemaphore;
			VkSemaphore SubmitSemaphore;

			CommandBufferVulkan* CommandBuffer;
		} PerFrameData[MaxFramesInFlight];

		struct PipelineDescriptorSetData
		{
			VkDescriptorSet DescriptorSets[16] {0};
		};

		// TODO
		struct
		{
			VkDescriptorSetLayout VerticesLayout;
			VkDescriptorSetLayout IndicesLayout;
			VkDescriptorSetLayout UVLayout;
			VkDescriptorSetLayout NormalLayout;
			VkDescriptorSetLayout TextureLayout;
			VkDescriptorSetLayout MaterialLayout;
			VkDescriptorSetLayout LightLayout;
		} GPUSceneLayout;

		// TODO
		struct
		{
			VkDescriptorSet VerticesSet;
			VkDescriptorSet IndicesSet;
			VkDescriptorSet UVSet;
			VkDescriptorSet NormalSet;
			VkDescriptorSet TextureSet;
			VkDescriptorSet MaterialSet;
			VkDescriptorSet LightSet;
		} GPUSceneData;

		std::unordered_map<VkPipeline, PipelineDescriptorSetData> DescriptorSets[MaxFramesInFlight];

		int CurrentPerFrameData = 0;
	};

	// TODO: Separate build, setup and execution contexts?
	struct RenderGraphContext
	{
		VkRenderPass VulkanRenderPass; // TODO: remove
		SPtr<DeviceVulkan> Device;
		SPtr<GPUScene> Scene;
		CommandBufferVulkan* CommandBuffer;
		RenderGraphData& RenderData;
		// RenderGraph& Graph;

		// TODO: better system
		Buffer* GetOutputBuffer(const std::string& Name, const BufferDesc& Desc);
		Buffer* GetInputBuffer(const std::string& Name);

		Texture2* GetRenderTarget(const std::string& Name, const TextureDesc2& Desc);
		Texture2* GetInputTexture(const std::string& Name);

		VkDescriptorSet GetDescriptorSet(const ComputePipeline* Pipeline, int Index);
		VkDescriptorSet GetDescriptorSet(const GraphicsPipeline* Pipeline, int Index);
		VkDescriptorSet GetDescriptorSet(const RayTracingPipeline* Pipeline, int Index);

		void BindGPUScene(const GraphicsPipeline* Pipeline);
		void BindGPUScene(const RayTracingPipeline* Pipeline);
	};

	class RenderPass
	{
	public:
		static constexpr const char* FinalColorOutput = "FinalColor";
	public:
		std::string Name;
		bool IsGraphicsPass = false;
		bool ClearColor = true;
	public:
		RenderPass() {}
		RenderPass(std::string_view Name) : Name(Name) {}

		void AddOutputRenderTarget(const AttachmentDesc& Desc)
		{
			RenderTargets.push_back(Desc);
		}

		// TODO: reduce
		virtual void Setup(RenderGraphContext& Context) = 0;
		virtual void PreExecute(RenderGraphContext& Context) {}
		virtual void Execute(RenderGraphContext& Context) = 0;

	private:
		friend RenderGraph;

		VkRenderPass VulkanRenderPass =  NULL;
		VkFramebuffer VulkanFramebuffers[16]{}; // TODO

		// std::vector<RenderGraphResourceId> Inputs;
		std::vector<AttachmentDesc> RenderTargets;
	};

	class RenderGraph
	{
	public:
		RenderGraph(SPtr<DeviceVulkan> Device, SPtr<GPUScene> Scene);

		void AddRenderPass(RenderPass* Pass);

		void Build();
		void Execute(SwapchainVulkan* Swapchain);

	private:
		std::vector<RenderPass*> RenderPasses;		

		SPtr<DeviceVulkan> Device;
		SPtr<GPUScene> Scene;

		RenderGraphData RenderData;

		VkRenderPass BlankPass; // TODO
	};

}
