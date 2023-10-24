#pragma once

#include <Core/Core.h>
#include <Graphics/Core/GraphicsCore.h>
#include <Graphics/Vulkan/DeviceVulkan.h>
#include <Profiling/Profiling.h>
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

DECLARE_CPU_PROFILING_COUNTER(Counter_RenderGraphClear);
DECLARE_CPU_PROFILING_COUNTER(Counter_RenderGraphBuild);
DECLARE_CPU_PROFILING_COUNTER(Counter_RenderGraphExecute);

namespace Columbus
{

	// forward declarations
	class RenderGraph;
	class RenderGraphContext;
	struct RenderGraphTexture;

	using RenderGraphExecutionFunc = std::function<void(RenderGraphContext&)>;
	using RenderGraphTextureRef = int; // TODO: unify with Id
	using RenderGraphTextureId = int;
	using RenderGraphPassId = int;

	constexpr int MaxFramesInFlight = 3;

	struct RenderGraphData
	{
		struct
		{
			SPtr<FenceVulkan> Fence;
			VkSemaphore ImageSemaphore;
			VkSemaphore SubmitSemaphore;

			CommandBufferVulkan* CommandBuffer;
		} PerFrameData[MaxFramesInFlight];

		// TODO: move to shader caching system
		struct PipelineDescriptorSetData
		{
			VkDescriptorSet DescriptorSets[16] {0};
		};

		// TODO: move to GPUScene
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

		// TODO: move to GPUScene
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

		// TODO: move to shader caching system
		std::unordered_map<VkPipeline, PipelineDescriptorSetData> DescriptorSets[MaxFramesInFlight];

		int CurrentPerFrameData = 0;

		// TODO: View-related
		uint32_t CurrentSwapchainImageIndex = 0;
		iVector2 CurrentSwapchainSize{-1};
		Texture2* SwapchainImage;
	};

	struct RenderGraphContext
	{
		VkRenderPass VulkanRenderPass; // TODO: remove
		RenderGraphPassId CurrentPass;
		SPtr<DeviceVulkan> Device;
		SPtr<GPUScene> Scene;
		CommandBufferVulkan* CommandBuffer;
		RenderGraphData& RenderData;
		RenderGraph& Graph;

		// TODO: separate shader binding system?
		VkDescriptorSet GetDescriptorSet(const ComputePipeline* Pipeline, int Index);
		VkDescriptorSet GetDescriptorSet(const GraphicsPipeline* Pipeline, int Index);
		VkDescriptorSet GetDescriptorSet(const RayTracingPipeline* Pipeline, int Index);

		void BindGPUScene(const GraphicsPipeline* Pipeline);
		void BindGPUScene(const RayTracingPipeline* Pipeline);

		SPtr<Texture2> GetRenderGraphTexture(RenderGraphTextureRef Ref);
	};

	struct RenderPassAttachment
	{
		AttachmentLoadOp LoadOp;
		RenderGraphTextureRef Texture;
		AttachmentClearValue ClearValue;

		bool operator==(const RenderPassAttachment&) const = default;
	};

	struct RenderPassParameters
	{
		static constexpr int ColorAttachmentsCount = 8;

		std::optional<RenderPassAttachment> ColorAttachments[ColorAttachmentsCount];
		std::optional<RenderPassAttachment> DepthStencilAttachment;

		// if you want to create render pass externally and use it in the graph
		VkRenderPass ExternalRenderPass = NULL;

		bool operator==(const RenderPassParameters& Other) const = default;
	};

	struct HashRenderPassParameters
	{
		size_t operator()(const RenderPassParameters& Params) const;
	};

	enum class RenderGraphPassType
	{
		Raster,
		Compute,
		// TODO: AsyncCompute
	};

	struct RenderGraphTexture
	{
		SPtr<Texture2> Texture;
		TextureDesc2 Desc;

		std::string DebugName;
		RenderGraphTextureId Id;

		// information about adjacent passes, needed to setup barriers
		// TODO: better solution to get a specific dependency (edge)?
		int Writer = -1; // can only be one writer
		fixed_vector<int, 32> Readers;

		// resource lifetime information
		RenderGraphPassId FirstUsage = INT_MAX;
		RenderGraphPassId LastUsage = INT_MIN;

		// Passes can overwrite a texture, incrementing a version
		int Version = -1;

		RenderGraphTexture(const TextureDesc2& Desc, std::string_view Name, RenderGraphTextureId Id)
			: Desc(Desc), DebugName(Name), Id(Id) {}
	};

	struct RenderPassTextureDependency
	{
		RenderGraphTextureId Texture;
		int Version = -1;

		// TODO?
		VkAccessFlags Access;
		VkPipelineStageFlags Stage;
	};

	struct RenderPassDependencies
	{
		void Write(RenderGraphTextureId Texture, VkAccessFlags Access, VkPipelineStageFlags Stage)
		{
			TextureWriteResources.push_back(RenderPassTextureDependency { Texture, -1, Access, Stage });
		}

		void Read(RenderGraphTextureId Texture, VkAccessFlags Access, VkPipelineStageFlags Stage)
		{
			TextureReadResources.push_back(RenderPassTextureDependency { Texture, -1, Access, Stage });
		}
	private:
		fixed_vector<RenderPassTextureDependency, 32> TextureWriteResources; // TODO: more efficient data storage
		fixed_vector<RenderPassTextureDependency, 32> TextureReadResources;  // TODO: more efficient data storage

		friend RenderGraph;
		friend RenderGraphContext;
	};

	struct RenderGraphTextureBarrier
	{
		RenderGraphTextureId Texture;
		RenderPassTextureDependency Writer; // (to extract vk flags from)
		RenderPassTextureDependency Reader; // (to extract vk flags from)
	};

	struct RenderPass
	{
		std::string Name;
		RenderGraphPassId Id; // index to internal Passes array
		RenderGraphPassType Type;
		RenderPassParameters Parameters;
		RenderPassDependencies Dependencies;
		RenderGraphExecutionFunc ExecutionFunc;

		fixed_vector<RenderGraphTextureBarrier, 64> TextureBarriers; // TODO: more efficient data storage

		VkRenderPass VulkanRenderPass = NULL;
		VkFramebuffer VulkanFramebuffer = NULL;
	};

	struct RenderGraphFramebufferVulkan
	{
		VkFramebuffer VulkanFramebuffer;
		iVector2 Size;
	};

	struct RenderGraphPooledTexture
	{
		SPtr<Texture2> Texture;
		bool Used = false;

		void Clear()
		{
			Used = false;
		}
	};

	// valid only while render graph is valid
	struct RenderGraphDebugInformation
	{
		std::span<RenderPass> Passes;
		std::span<RenderGraphTexture> Textures;
	};

	class RenderGraph
	{
	public:
		RenderGraph(SPtr<DeviceVulkan> Device, SPtr<GPUScene> Scene);

		RenderGraphTextureRef CreateTexture(const TextureDesc2& Desc, const char* Name);
		// TODO: CreateBuffer

		RenderGraphTextureRef GetSwapchainTexture();

		void AddPass(const std::string& Name, RenderGraphPassType Type, RenderPassParameters Parameters, RenderPassDependencies Dependencies, RenderGraphExecutionFunc ExecuteCallback);

		void Clear();
		void Execute(SwapchainVulkan* Swapchain);

		// These functions should be used after graph was built
		std::string ExportGraphviz(); // exports graphviz dot format
		void PrintDebugInformation();
		void GetDebugInformation(RenderGraphDebugInformation& Info);

	private:
		void Build(Texture2* SwapchainImage);

		void AllocateTexture(RenderGraphTexture& Texture);

		VkRenderPass GetOrCreateVulkanRenderPass(RenderPass& Pass);
		VkFramebuffer GetOrCreateVulkanFramebuffer(RenderPass& Pass, Texture2* SwapchainImage);

	private:
		std::vector<RenderPass> Passes;
		std::vector<RenderGraphTexture> Textures; // textures that are used in the current graph

		std::unordered_map<RenderPassParameters, VkRenderPass, HashRenderPassParameters> MapOfVulkanRenderPasses;
		std::unordered_map<VkRenderPass, RenderGraphFramebufferVulkan> MapOfVulkanFramebuffers[MaxFramesInFlight];

		std::unordered_map<TextureDesc2, std::vector<RenderGraphPooledTexture>, HashTextureDesc2> TextureResourcePool;

		SPtr<DeviceVulkan> Device;
		SPtr<GPUScene> Scene;

		RenderGraphData RenderData;

	private:
		static constexpr RenderGraphTextureRef SwapchainId = -1;

		friend RenderGraphContext;
	};

}
