#pragma once

#include <Graphics/Core/ShaderBinding.h>
#include <vulkan/vulkan.h>

#include <cstdint>

namespace Columbus
{
	struct ShaderBinderBackendStateVulkan : ShaderBinderBackendState
	{
		static constexpr int MaxDescriptorSets = 16;

		// Borrowed per Shader::Bind traversal plus Flush. RenderGraphContext
		// owns descriptor allocation/cache policy and descriptor lifetime.
		VkDescriptorSet Sets[MaxDescriptorSets] = {};
		uint32_t UsedMask = 0;
	};
}
