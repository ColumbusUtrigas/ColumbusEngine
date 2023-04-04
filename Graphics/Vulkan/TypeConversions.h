#pragma once

#include <Graphics/Buffer.h>
#include <Graphics/Types.h>
#include <Common/Image/Image.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace Columbus
{

	static VkPolygonMode FillModeToVK(FillMode fill)
	{
		switch (fill)
		{
			case FillMode::Solid:     return VK_POLYGON_MODE_FILL;
			case FillMode::Wireframe: return VK_POLYGON_MODE_LINE;
		}
	}

	static VkCullModeFlagBits CullModeToVK(CullMode cull)
	{
		switch (cull)
		{
			case CullMode::No:    return VK_CULL_MODE_NONE;
			case CullMode::Front: return VK_CULL_MODE_FRONT_BIT;
			case CullMode::Back:  return VK_CULL_MODE_BACK_BIT;
		}
	}

	static VkFormat TextureFormatToVK(TextureFormat format)
	{
		switch (format)
		{
			case TextureFormat::RGB8: return VK_FORMAT_R8G8B8_SRGB;
			case TextureFormat::RGBA8: return VK_FORMAT_R8G8B8A8_SRGB;
			case TextureFormat::DXT1: return VK_FORMAT_BC1_RGB_SRGB_BLOCK;
		}
	}

	static VkBufferUsageFlags BufferTypeToVK(BufferType type)
	{
		switch (type)
		{
			case BufferType::Array:    return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
			case BufferType::Index:    return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
			case BufferType::Constant: return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			case BufferType::UAV:      return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		}
	}

}
