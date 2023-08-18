#pragma once

#include <Core/Core.h>
#include <Graphics/Core/GraphicsCore.h>
#include <Graphics/Vulkan/DeviceVulkan.h>
#include "GPUScene.h"
#include "Graphics/Vulkan/CommandBufferVulkan.h"
#include "Graphics/Vulkan/FenceVulkan.h"
#include "Graphics/Vulkan/SwapchainVulkan.h"
#include <memory>
#include <vector>
#include <unordered_map>
#include <functional>
#include <optional>

#include <vulkan/vulkan.h>

namespace Columbus
{

	class RenderGraph;

	constexpr int MaxFramesInFlight = 3;

	struct RenderGraphData
	{
		std::unordered_map<std::string, Buffer*> Buffers; // TODO: remove legacy
		std::unordered_map<std::string, Texture2*> Textures; // TODO: remove legacy

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
		uint32_t CurrentSwapchainImageIndex = 0;
		iVector2 CurrentSwapchainSize{-1};
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

		// TODO: REMOVE LEGACY
		Buffer* GetOutputBuffer(const std::string& Name, const BufferDesc& Desc, void* InitialData = nullptr);
		Buffer* GetInputBuffer(const std::string& Name);

		// TODO: REMOVE LEGACY
		Texture2* GetRenderTarget(const std::string& Name, const TextureDesc2& Desc);
		Texture2* GetInputTexture(const std::string& Name);

		// TODO: separate shader binding system?
		VkDescriptorSet GetDescriptorSet(const ComputePipeline* Pipeline, int Index);
		VkDescriptorSet GetDescriptorSet(const GraphicsPipeline* Pipeline, int Index);
		VkDescriptorSet GetDescriptorSet(const RayTracingPipeline* Pipeline, int Index);

		void BindGPUScene(const GraphicsPipeline* Pipeline);
		void BindGPUScene(const RayTracingPipeline* Pipeline);
	};

	using RenderGraphExecutionFunc = std::function<void(RenderGraphContext&)>;
	using RenderGraphTextureRef = SPtr<Texture2>;

	struct RenderPassAttachment
	{
		AttachmentLoadOp LoadOp;
		Texture2* Texture;
		AttachmentClearValue ClearValue;

		bool operator==(const RenderPassAttachment&) const = default;
	};

	struct RenderPassParameters
	{
		static constexpr int ColorAttachmentsCount = 8;

		std::optional<RenderPassAttachment> ColorAttachments[ColorAttachmentsCount];
		std::optional<RenderPassAttachment> DepthStencilAttachment;

		bool operator==(const RenderPassParameters& Other) const = default;
	};

	struct HashRenderPassParameters
	{
		size_t operator()(const RenderPassParameters& Params) const;
	};

	// TODO: remove legacy
	class RenderPass
	{
	public:
		static constexpr const char* FinalColorOutput = "FinalColor";

		using TExecutionFunc = std::function<void(RenderGraphContext&)>;
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
		virtual TExecutionFunc Execute2(RenderGraphContext& Context) { return [](RenderGraphContext&){}; }

	private:
		friend RenderGraph;

		VkRenderPass VulkanRenderPass =  NULL;
		VkFramebuffer VulkanFramebuffers[16]{}; // TODO

		// std::vector<RenderGraphResourceId> Inputs;
		std::vector<AttachmentDesc> RenderTargets;
	};

	enum class RenderGraphPassType
	{
		Raster,
		Compute,
		// TODO: AsyncCompute
	};

	struct RenderPassTextureDependency
	{
		RenderGraphTextureRef Texture;

		// TODO?
		VkAccessFlags Access;
		VkPipelineStageFlags Stage;
	};

	struct RenderPassDependencies
	{
		void Write(RenderGraphTextureRef Texture, VkAccessFlags Access, VkPipelineStageFlags Stage)
		{
			TextureWriteResources.push_back(RenderPassTextureDependency { Texture, Access, Stage });
		}

		void Read(RenderGraphTextureRef Texture, VkAccessFlags Access, VkPipelineStageFlags Stage)
		{
			TextureReadResources.push_back(RenderPassTextureDependency { Texture, Access, Stage });
		}
	private:
		fixed_vector<RenderPassTextureDependency, 32> TextureWriteResources;
		fixed_vector<RenderPassTextureDependency, 32> TextureReadResources;

		friend class RenderGraph;
	};

	struct RenderPass2
	{
		std::string Name;
		RenderGraphPassType Type;
		RenderPassParameters Parameters;
		RenderPassDependencies Dependencies;
		RenderGraphExecutionFunc ExecutionFunc;

		VkRenderPass VulkanRenderPass = NULL;
		VkFramebuffer VulkanFramebuffer = NULL;
	};

	struct RenderGraphFramebufferVulkan
	{
		VkFramebuffer VulkanFramebuffer;
		iVector2 Size;
	};

	class RenderGraph
	{
	public:
		RenderGraph(SPtr<DeviceVulkan> Device, SPtr<GPUScene> Scene);

		RenderGraphTextureRef CreateTexture(const TextureDesc2& Desc, const char* Name);

		// TODO: legacy
		void AddRenderPass(RenderPass* Pass);

		Texture2* GetSwapchainTexture();

		void AddPass(const std::string& Name, RenderGraphPassType Type, RenderPassParameters Parameters, RenderPassDependencies Dependencies, RenderGraphExecutionFunc ExecuteCallback);

		void Clear();
		void Execute(SwapchainVulkan* Swapchain);

	private:
		void Build(Texture2* SwapchainImage);

		VkRenderPass GetOrCreateVulkanRenderPass(RenderPass2& Pass);
		VkFramebuffer GetOrCreateVulkanFramebuffer(RenderPass2& Pass, Texture2* SwapchainImage);

	private:
		std::vector<RenderPass*> RenderPasses;
		std::vector<RenderPass2> Passes;

		std::unordered_map<RenderPassParameters, VkRenderPass, HashRenderPassParameters> MapOfVulkanRenderPasses;
		std::unordered_map<VkRenderPass, RenderGraphFramebufferVulkan> MapOfVulkanFramebuffers[MaxFramesInFlight];

		// TODO: handles + array?
		std::unordered_map<TextureDesc2, std::vector<RenderGraphTextureRef>, HashTextureDesc2> TextureResourceTable;

		SPtr<DeviceVulkan> Device;
		SPtr<GPUScene> Scene;

		RenderGraphData RenderData;
	};

}
