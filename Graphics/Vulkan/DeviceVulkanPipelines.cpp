#include "DeviceVulkan.h"

#include "ComputePipelineVulkan.h"
#include "GraphicsPipelineVulkan.h"
#include "RayTracingPipelineVulkan.h"

#include <vulkan/vulkan.hpp>

namespace Columbus
{

	ComputePipeline* DeviceVulkan::CreateComputePipeline(const ComputePipelineDesc &Desc)
	{
		auto result = new ComputePipelineVulkan(Desc);

		ShaderStageCompilationResult ComputeStage = BuildShaderStage(Desc.CS, "NONAME FIXME");

		VkPipelineShaderStageCreateInfo stages[] = {
			ComputeStage.ShaderStageInfo
		};

		// TODO: Caching scheme, unification with other pipeline types, other stages support
		for (int i = 0; i < ComputeStage.Spirv.descriptorSets.size(); i++)
		{
			VK_CHECK(vkCreateDescriptorSetLayout(_Device, &ComputeStage.Spirv.descriptorSets[i], nullptr, &result->setLayouts[i]));
		}

		VkPipelineLayoutCreateInfo layoutInfo;
		layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutInfo.pNext = nullptr;
		layoutInfo.flags = 0;
		layoutInfo.setLayoutCount = ComputeStage.Spirv.descriptorSets.size();
		layoutInfo.pSetLayouts = result->setLayouts;
		layoutInfo.pushConstantRangeCount = 0;
		layoutInfo.pPushConstantRanges = nullptr;

		VK_CHECK(vkCreatePipelineLayout(_Device, &layoutInfo, nullptr, &result->layout));

		VkComputePipelineCreateInfo info;
		info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = 0;
		info.stage = ComputeStage.ShaderStageInfo;
		info.layout = result->layout;
		info.basePipelineHandle = nullptr;
		info.basePipelineIndex = -1;

		VK_CHECK(vkCreateComputePipelines(_Device, nullptr, 1, &info, nullptr, &result->pipeline));

		return result;
	}

	RayTracingPipeline* DeviceVulkan::CreateRayTracingPipeline(const RayTracingPipelineDesc &Desc)
	{
		auto result = new RayTracingPipelineVulkan(Desc);

		ShaderStageCompilationResult GenStage = BuildShaderStage(Desc.RayGen, "NONAME FIXME");
		ShaderStageCompilationResult MissStage = BuildShaderStage(Desc.Miss, "NONAME FIXME");
		ShaderStageCompilationResult ClosestHitStage = BuildShaderStage(Desc.ClosestHit, "NONAME FIXME");
		std::vector<ShaderStageCompilationResult> compiledStages = { GenStage, MissStage, ClosestHitStage };

		VkPipelineShaderStageCreateInfo stages[] = {
			GenStage.ShaderStageInfo,
			MissStage.ShaderStageInfo,
			ClosestHitStage.ShaderStageInfo
		};

		// Concat all push constants and descriptor sets
		std::vector<VkPushConstantRange> pushConstants;
		std::vector<VkDescriptorSetLayoutCreateInfo> descriptorSets;

		for (auto& stage : compiledStages)
		{
			for (auto& pushConstant : stage.Spirv.pushConstants)
			{
				pushConstants.push_back(pushConstant);
			}

			for (auto& descriptorSet : stage.Spirv.descriptorSets)
			{
				descriptorSets.push_back(descriptorSet);
			}
		}

		// TODO: Caching scheme, unification with other pipeline types, other stages support
		for (int i = 0; i < descriptorSets.size(); i++)
		{
			VK_CHECK(vkCreateDescriptorSetLayout(_Device, &descriptorSets[i], nullptr, &result->setLayouts[i]));
		}

		VkPipelineLayoutCreateInfo layoutInfo;
		layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutInfo.pNext = nullptr;
		layoutInfo.flags = 0;
		layoutInfo.setLayoutCount = descriptorSets.size();
		layoutInfo.pSetLayouts = result->setLayouts;
		layoutInfo.pushConstantRangeCount = pushConstants.size();
		layoutInfo.pPushConstantRanges = pushConstants.data();

		VK_CHECK(vkCreatePipelineLayout(_Device, &layoutInfo, nullptr, &result->layout));

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
		info.maxPipelineRayRecursionDepth = 2;
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

		result->RayGenSBT = CreateBuffer(handleSize, shaderHandleStorage.data(), Columbus::BufferType::ShaderBindingTable, true);
		result->MissSBT = CreateBuffer(handleSize, shaderHandleStorage.data() + handleSizeAligned, Columbus::BufferType::ShaderBindingTable, true);
		result->HitSBT = CreateBuffer(handleSize, shaderHandleStorage.data() + handleSizeAligned * 2, Columbus::BufferType::ShaderBindingTable, true);

		vk::Device dev(_Device);

		result->RayGenRegionSBT = vk::StridedDeviceAddressRegionKHR(dev.getBufferAddress({ result->RayGenSBT.Buffer }), handleSizeAligned, handleSizeAligned);
		result->MissRegionSBT = vk::StridedDeviceAddressRegionKHR(dev.getBufferAddress({ result->MissSBT.Buffer }), handleSizeAligned, handleSizeAligned);
		result->HitRegionSBT = vk::StridedDeviceAddressRegionKHR(dev.getBufferAddress({ result->HitSBT.Buffer }), handleSizeAligned, handleSizeAligned);
		result->CallableRegionSBT = {};

		return result;
	}

}
