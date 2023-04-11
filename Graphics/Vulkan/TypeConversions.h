#pragma once

#include "Core/Assert.h"
#include <Graphics/Texture.h>
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

	static VkImageType TextureTypeToImageTypeVk(TextureType type)
	{
		switch (type)
		{
			case TextureType::Texture1DArray:
			case TextureType::Texture1D: return VK_IMAGE_TYPE_1D;

			case TextureType::Texture2DArray:
			case TextureType::Texture2D: return VK_IMAGE_TYPE_2D;

			case TextureType::Texture3D: return VK_IMAGE_TYPE_3D;

			case TextureType::TextureCubeArray:
			case TextureType::TextureCube: return VK_IMAGE_TYPE_2D;

			default: COLUMBUS_ASSERT(false);
		}
	}

	static VkImageViewType TextureTypeToViewTypeVk(TextureType type)
	{
		switch (type)
		{
			case TextureType::Texture1D:        return VK_IMAGE_VIEW_TYPE_1D;
			case TextureType::Texture2D:        return VK_IMAGE_VIEW_TYPE_2D;
			case TextureType::Texture3D:        return VK_IMAGE_VIEW_TYPE_3D;
			case TextureType::TextureCube:      return VK_IMAGE_VIEW_TYPE_CUBE;
			case TextureType::Texture1DArray:   return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
			case TextureType::Texture2DArray:   return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
			case TextureType::TextureCubeArray: return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
			default: COLUMBUS_ASSERT(false);
		}
	}

	static VkFormat TextureFormatToVK(TextureFormat format)
	{
		switch (format)
		{
			case TextureFormat::RGB8: return VK_FORMAT_R8G8B8_UNORM;
			case TextureFormat::RGBA8: return VK_FORMAT_R8G8B8A8_UNORM;
			case TextureFormat::DXT1: return VK_FORMAT_BC1_RGB_SRGB_BLOCK;
			case TextureFormat::DXT5: return VK_FORMAT_BC5_UNORM_BLOCK;
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
			case BufferType::AccelerationStructureStorage: return VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
			case BufferType::AccelerationStructureInput: return VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
			case BufferType::ShaderBindingTable: return VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR;
		}
	}

	static VkShaderStageFlagBits ShaderTypeToVk(ShaderType type)
	{
		int64_t result = 0;

		if ((type & ShaderType::Vertex) != 0)   result |= VK_SHADER_STAGE_VERTEX_BIT;
		if ((type & ShaderType::Pixel) != 0)    result |= VK_SHADER_STAGE_FRAGMENT_BIT;
		if ((type & ShaderType::Hull)   != 0)   COLUMBUS_ASSERT_MESSAGE(false, "Tesselation isn't supported");
		if ((type & ShaderType::Domain) != 0)   COLUMBUS_ASSERT_MESSAGE(false, "Tesselation isn't supported");
		if ((type & ShaderType::Geometry) != 0) result |= VK_SHADER_STAGE_GEOMETRY_BIT;

		if ((type & ShaderType::Compute) != 0) result |= VK_SHADER_STAGE_COMPUTE_BIT;

		if ((type & ShaderType::Raygen) != 0)       result |= VK_SHADER_STAGE_RAYGEN_BIT_KHR;
		if ((type & ShaderType::Miss) != 0)         result |= VK_SHADER_STAGE_MISS_BIT_KHR;
		if ((type & ShaderType::Anyhit) != 0)       result |= VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
		if ((type & ShaderType::ClosestHit) != 0)   result |= VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
		if ((type & ShaderType::Intersection) != 0) result |= VK_SHADER_STAGE_INTERSECTION_BIT_KHR;

		return static_cast<VkShaderStageFlagBits>(result);
	}

}
