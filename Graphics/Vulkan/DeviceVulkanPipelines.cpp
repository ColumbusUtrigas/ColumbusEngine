#include "DeviceVulkan.h"
#include "PipelinesVulkan.h"

#include <vulkan/vulkan.hpp>

namespace Columbus
{

	ComputePipeline* DeviceVulkan::CreateComputePipeline(const ComputePipelineDesc &Desc)
	{
		auto result = new ComputePipelineVulkan(Desc);

		ShaderStageBuildResultVulkan ComputeStage = ShaderStageBuild_VK(Desc.CS, Desc.Name.c_str(), _Device);
		std::vector<ShaderStageBuildResultVulkan> CompiledStages = { ComputeStage };

		VkPipelineShaderStageCreateInfo stages[] = {
			ComputeStage.ShaderStageInfo
		};

		result->layout = _CreatePipelineLayout(CompiledStages, result->SetLayouts);

		VkComputePipelineCreateInfo info;
		info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = 0;
		info.stage = ComputeStage.ShaderStageInfo;
		info.layout = result->layout;
		info.basePipelineHandle = nullptr;
		info.basePipelineIndex = -1;

		VK_CHECK(vkCreateComputePipelines(_Device, nullptr, 1, &info, nullptr, &result->pipeline));

		if (!Desc.Name.empty())
		{
			SetDebugName(result, Desc.Name.c_str());
		}

		return result;
	}

	GraphicsPipeline* DeviceVulkan::CreateGraphicsPipeline(const GraphicsPipelineDesc& Desc, VkRenderPass RenderPass)
	{
		auto vs = ShaderStageBuild_VK(Desc.VS, Desc.Name, _Device);
		auto ps = ShaderStageBuild_VK(Desc.PS, Desc.Name, _Device);

		std::vector<ShaderStageBuildResultVulkan> compiledStages = { vs, ps };
		VkPipelineShaderStageCreateInfo stages[] = { vs.ShaderStageInfo, ps.ShaderStageInfo };

		auto pipeline = new GraphicsPipelineVulkan(Desc);

		fixed_vector<VkVertexInputBindingDescription, 16> inputBindings;
		for (int i = 0; i < Desc.layout.Elements.size(); i++)
		{
			auto element = Desc.layout.Elements[i];

			VkVertexInputBindingDescription vertexInputBinding;
			vertexInputBinding.binding = element.Slot;
			vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // TODO
			vertexInputBinding.stride = sizeof(float) * element.Components; // TODO

			inputBindings.push_back(vertexInputBinding);
		}

		// TODO
		fixed_vector<VkVertexInputAttributeDescription, 16> inputAttribute;
		for (int i = 0; i < Desc.layout.Elements.size(); i++)
		{
			auto element = Desc.layout.Elements[i];

			VkVertexInputAttributeDescription attribute;

			attribute.binding = element.Slot;
			attribute.location = element.SemanticIndex;
			attribute.offset = 0;

			// TODO
			switch (element.Components)
			{
			case 2: attribute.format = VK_FORMAT_R32G32_SFLOAT;    break;
			case 3: attribute.format = VK_FORMAT_R32G32B32_SFLOAT; break;
			}

			inputAttribute.push_back(attribute);
		}

		VkPipelineVertexInputStateCreateInfo vertexInputState;
		vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputState.pNext = nullptr;
		vertexInputState.flags = 0;
		vertexInputState.vertexBindingDescriptionCount = inputBindings.size();
		vertexInputState.pVertexBindingDescriptions = inputBindings.data();
		vertexInputState.vertexAttributeDescriptionCount = inputAttribute.size();
		vertexInputState.pVertexAttributeDescriptions = inputAttribute.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState;
		inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyState.pNext = nullptr;
		inputAssemblyState.flags = 0;
		inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblyState.primitiveRestartEnable = false;

		VkPipelineTessellationStateCreateInfo tesselationState;
		tesselationState.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
		tesselationState.pNext = nullptr;
		tesselationState.flags = 0;
		tesselationState.patchControlPoints = 1;

		VkViewport viewport;
		viewport.x = 0;
		viewport.y = 0;
		viewport.width = 1280; // TODO
		viewport.height = 720; // TODO
		viewport.minDepth = 0;
		viewport.maxDepth = 1;

		VkRect2D scissor;
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		scissor.extent.width = 1280; // TODO
		scissor.extent.height = 720; // TODO

		VkPipelineViewportStateCreateInfo viewportState;
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.pNext = nullptr;
		viewportState.flags = 0;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterState;
		rasterState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterState.pNext = nullptr;
		rasterState.flags = 0;
		rasterState.depthClampEnable = false;
		rasterState.rasterizerDiscardEnable =  false;
		rasterState.polygonMode = FillModeToVK(Desc.rasterizerState.Fill);
		rasterState.cullMode = CullModeToVK(Desc.rasterizerState.Cull);
		rasterState.frontFace = Desc.rasterizerState.FrontCounterClockwise ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE;
		rasterState.depthBiasEnable = false;
		rasterState.depthBiasConstantFactor = Desc.rasterizerState.DepthBias;
		rasterState.depthBiasClamp = Desc.rasterizerState.DepthBiasClamp;
		rasterState.depthBiasSlopeFactor = Desc.rasterizerState.SlopeScaledDepthBias;
		rasterState.lineWidth = 1;

		VkPipelineMultisampleStateCreateInfo multisampleState;
		multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleState.pNext = nullptr;
		multisampleState.flags = 0;
		multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampleState.sampleShadingEnable = false;
		multisampleState.minSampleShading = 0;
		multisampleState.pSampleMask = nullptr;
		multisampleState.alphaToCoverageEnable = false;
		multisampleState.alphaToOneEnable = false;

		VkPipelineDepthStencilStateCreateInfo depthStencilState;
		depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilState.pNext = nullptr;
		depthStencilState.flags = 0;
		depthStencilState.depthTestEnable = Desc.depthStencilState.DepthEnable;
		depthStencilState.depthWriteEnable = Desc.depthStencilState.DepthWriteMask;
		depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS; // TODO
		depthStencilState.depthBoundsTestEnable = false; // TODO
		depthStencilState.stencilTestEnable = false; // TODO
		depthStencilState.front = {}; // TODO
		depthStencilState.back = {}; // TODO
		depthStencilState.minDepthBounds = 0; // TODO
		depthStencilState.maxDepthBounds = 1; // TODO

		fixed_vector<VkPipelineColorBlendAttachmentState, 16> Attachments;
		for (int i = 0; i < Desc.blendState.RenderTargets.size(); i++)
		{
			VkPipelineColorBlendAttachmentState Attachment;
			Attachment.blendEnable = Desc.blendState.RenderTargets[i].BlendEnable;
			Attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO; // TODO
			Attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // TODO
			Attachment.colorBlendOp = VK_BLEND_OP_ADD; // TODO
			Attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // TODO
			Attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // TODO
			Attachment.alphaBlendOp = VK_BLEND_OP_ADD; // TODO
			Attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT; // TODO

			Attachments.push_back(Attachment);
		}

		VkPipelineColorBlendStateCreateInfo blendState;
		blendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		blendState.pNext = nullptr;
		blendState.flags = 0;
		blendState.logicOpEnable = false;
		blendState.logicOp = VK_LOGIC_OP_COPY;
		blendState.attachmentCount = Attachments.size();
		blendState.pAttachments = Attachments.data();
		blendState.blendConstants[0] = 0;
		blendState.blendConstants[1] = 0;
		blendState.blendConstants[2] = 0;
		blendState.blendConstants[3] = 0;

		VkPipelineDynamicStateCreateInfo dynamicState;
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.pNext = nullptr;
		dynamicState.flags = 0;
		dynamicState.dynamicStateCount = 0;
		dynamicState.pDynamicStates = nullptr;

		pipeline->layout = _CreatePipelineLayout(compiledStages, pipeline->SetLayouts);

		VkGraphicsPipelineCreateInfo info;
		info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = 0;
		info.stageCount = 2;
		info.pStages = stages;
		info.pVertexInputState = &vertexInputState;
		info.pInputAssemblyState = &inputAssemblyState;
		info.pTessellationState = &tesselationState;
		info.pViewportState = &viewportState;
		info.pRasterizationState = &rasterState;
		info.pMultisampleState = &multisampleState;
		info.pDepthStencilState = &depthStencilState;
		info.pColorBlendState = &blendState;
		info.pDynamicState = &dynamicState;
		info.layout = pipeline->layout;
		info.renderPass = RenderPass;
		info.subpass = 0;
		info.basePipelineHandle = nullptr;
		info.basePipelineIndex = 0;

		VK_CHECK(vkCreateGraphicsPipelines(_Device, nullptr, 1, &info, nullptr, &pipeline->pipeline));

		if (!Desc.Name.empty())
		{
			SetDebugName(pipeline, Desc.Name.c_str());
		}

		return pipeline;
	}

	RayTracingPipeline* DeviceVulkan::CreateRayTracingPipeline(const RayTracingPipelineDesc &Desc)
	{
		auto result = new RayTracingPipelineVulkan(Desc);

		ShaderStageBuildResultVulkan GenStage = ShaderStageBuild_VK(Desc.RayGen, Desc.Name.c_str(), _Device);
		ShaderStageBuildResultVulkan MissStage = ShaderStageBuild_VK(Desc.Miss, Desc.Name.c_str(), _Device);
		ShaderStageBuildResultVulkan ClosestHitStage = ShaderStageBuild_VK(Desc.ClosestHit, Desc.Name.c_str(), _Device);
		std::vector<ShaderStageBuildResultVulkan> compiledStages = { GenStage, MissStage, ClosestHitStage };

		VkPipelineShaderStageCreateInfo stages[] = {
			GenStage.ShaderStageInfo,
			MissStage.ShaderStageInfo,
			ClosestHitStage.ShaderStageInfo
		};

		result->layout = _CreatePipelineLayout(compiledStages, result->SetLayouts);

		VkPipelineDynamicStateCreateInfo dynamicState;
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.pNext = nullptr;
		dynamicState.flags = 0;
		dynamicState.dynamicStateCount = 0;
		dynamicState.pDynamicStates = nullptr;

		// Build shader groups
		// TODO: build groups automatically based on used stages
		std::vector<VkRayTracingShaderGroupCreateInfoKHR> groups;

		{
			VkRayTracingShaderGroupCreateInfoKHR group;
			group.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
			group.pNext = nullptr;
			group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
			group.generalShader = 0;
			group.closestHitShader = VK_SHADER_UNUSED_KHR;
			group.anyHitShader = VK_SHADER_UNUSED_KHR;
			group.intersectionShader = VK_SHADER_UNUSED_KHR;
			group.pShaderGroupCaptureReplayHandle = nullptr;
			groups.push_back(group);
		}

		{
			VkRayTracingShaderGroupCreateInfoKHR group;
			group.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
			group.pNext = nullptr;
			group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
			group.generalShader = 1;
			group.closestHitShader = VK_SHADER_UNUSED_KHR;
			group.anyHitShader = VK_SHADER_UNUSED_KHR;
			group.intersectionShader = VK_SHADER_UNUSED_KHR;
			group.pShaderGroupCaptureReplayHandle = nullptr;
			groups.push_back(group);
		}

		{
			VkRayTracingShaderGroupCreateInfoKHR group;
			group.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
			group.pNext = nullptr;
			group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
			group.generalShader = VK_SHADER_UNUSED_KHR;
			group.closestHitShader = 2;
			group.anyHitShader = VK_SHADER_UNUSED_KHR;
			group.intersectionShader = VK_SHADER_UNUSED_KHR;
			group.pShaderGroupCaptureReplayHandle = nullptr;
			groups.push_back(group);
		}

		VkRayTracingPipelineCreateInfoKHR info;
		info.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
		info.pNext = nullptr;
		info.flags = 0;
		info.stageCount = 3;
		info.pStages = stages;
		info.groupCount = groups.size();
		info.pGroups = groups.data();
		info.maxPipelineRayRecursionDepth = Desc.MaxRecursionDepth;
		info.pLibraryInfo = nullptr;
		info.pLibraryInterface = nullptr;
		info.pDynamicState = &dynamicState;
		info.layout = result->layout;
		info.basePipelineHandle = nullptr;
		info.basePipelineIndex = -1;

		VkFunctions.vkCreateRayTracingPipelines(_Device, NULL, NULL, 1, &info, nullptr, &result->pipeline);

		// Build Shader Binding Table
		const uint32_t handleSize = _RayTracingProperties.shaderGroupHandleSize;
		const uint32_t handleSizeAligned = alignedSize(_RayTracingProperties.shaderGroupHandleSize, _RayTracingProperties.shaderGroupHandleAlignment);
		const uint32_t groupCount = 3;
		const uint32_t sbtSize = groupCount * handleSizeAligned;

		std::vector<uint8_t> shaderHandleStorage(sbtSize);
		VK_CHECK(VkFunctions.vkGetRayTracingShaderGroupHandles(_Device, result->pipeline, 0, groupCount, sbtSize, shaderHandleStorage.data()));

		result->RayGenSBT = CreateBuffer({handleSize, BufferType::ShaderBindingTable, true}, shaderHandleStorage.data());
		result->MissSBT = CreateBuffer({handleSize, BufferType::ShaderBindingTable, true}, shaderHandleStorage.data() + handleSizeAligned);
		result->HitSBT = CreateBuffer({handleSize, BufferType::ShaderBindingTable, true}, shaderHandleStorage.data() + handleSizeAligned * 2);

		vk::Device dev(_Device);

		result->RayGenRegionSBT = vk::StridedDeviceAddressRegionKHR(GetBufferDeviceAddress(result->RayGenSBT), handleSizeAligned, handleSizeAligned);
		result->MissRegionSBT = vk::StridedDeviceAddressRegionKHR(GetBufferDeviceAddress(result->MissSBT), handleSizeAligned, handleSizeAligned);
		result->HitRegionSBT = vk::StridedDeviceAddressRegionKHR(GetBufferDeviceAddress(result->HitSBT), handleSizeAligned, handleSizeAligned);
		result->CallableRegionSBT = {};

		if (!Desc.Name.empty())
		{
			SetDebugName(result, Desc.Name.c_str());
		}

		return result;
	}

}
