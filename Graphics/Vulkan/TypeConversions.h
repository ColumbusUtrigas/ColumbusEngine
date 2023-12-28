#pragma once

#include "Core/Assert.h"
#include "Graphics/Core/GraphicsCore.h"
#include "Graphics/Core/QueryPool.h"
#include <Common/Image/Image.h>
#include <vulkan/vulkan.h>
#include <cassert>
#include <vulkan/vulkan_core.h>

namespace Columbus
{

	static VkPolygonMode FillModeToVK(FillMode fill)
	{
		switch (fill)
		{
			case FillMode::Solid:     return VK_POLYGON_MODE_FILL;
			case FillMode::Wireframe: return VK_POLYGON_MODE_LINE;

			default: COLUMBUS_ASSERT(false);
		}
	}

	static VkCullModeFlagBits CullModeToVK(CullMode cull)
	{
		switch (cull)
		{
			case CullMode::No:    return VK_CULL_MODE_NONE;
			case CullMode::Front: return VK_CULL_MODE_FRONT_BIT;
			case CullMode::Back:  return VK_CULL_MODE_BACK_BIT;

			default: COLUMBUS_ASSERT(false);
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

	static VkImageCreateFlags TextureTypeToImageFlagsVk(TextureType type)
	{
		switch (type)
		{
			case TextureType::TextureCubeArray:
			case TextureType::TextureCube: return VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

			case TextureType::Texture1D:
			case TextureType::Texture2D:
			case TextureType::Texture3D:
			case TextureType::Texture1DArray:
			case TextureType::Texture2DArray: return 0;

			default: COLUMBUS_ASSERT(false);
		}
	}

	static VkFormat TextureFormatToVK(TextureFormat format)
	{
		switch (format)
		{
			case TextureFormat::R8: return VK_FORMAT_R8_UNORM;
			case TextureFormat::RG8: return VK_FORMAT_R8G8_UNORM;
			case TextureFormat::RGB8: return VK_FORMAT_R8G8B8_UNORM;
			case TextureFormat::RGBA8: return VK_FORMAT_R8G8B8A8_UNORM;

			case TextureFormat::RG16F: return VK_FORMAT_R16G16_SFLOAT;
			case TextureFormat::RGB16F: return VK_FORMAT_R16G16B16_SFLOAT;
			case TextureFormat::RGBA16F: return VK_FORMAT_R16G16B16A16_SFLOAT;

			case TextureFormat::RGBA32F: return VK_FORMAT_R32G32B32A32_SFLOAT;

			case TextureFormat::R8UInt:  return VK_FORMAT_R8_UINT;
			case TextureFormat::R32UInt: return VK_FORMAT_R32_UINT;

			case TextureFormat::BGRA8SRGB: return VK_FORMAT_B8G8R8A8_SRGB;

			case TextureFormat::R11G11B10F: return VK_FORMAT_B10G11R11_UFLOAT_PACK32;

			case TextureFormat::Depth24: return VK_FORMAT_D24_UNORM_S8_UINT;
			case TextureFormat::DXT1: return VK_FORMAT_BC1_RGB_SRGB_BLOCK;
			case TextureFormat::DXT5: return VK_FORMAT_BC5_UNORM_BLOCK;
			default: COLUMBUS_ASSERT(false);
		}
	}

	static VkImageAspectFlags TextureFormatToAspectMaskVk(TextureFormat format)
	{
		return ImageIsDepthFormat(format) ? (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT) : VK_IMAGE_ASPECT_COLOR_BIT;
	}

	static VkImageUsageFlags TextureUsageToImageUsageVk(TextureUsage usage)
	{
		switch (usage)
		{
			case TextureUsage::Sampled: return VK_IMAGE_USAGE_SAMPLED_BIT;
			case TextureUsage::Storage: return VK_IMAGE_USAGE_STORAGE_BIT;
			case TextureUsage::RenderTargetColor: return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			case TextureUsage::RenderTargetDepth: return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			default: COLUMBUS_ASSERT(false);
		}
	}

	static VkDescriptorType TextureUsageToVkDescriptorType(TextureUsage usage)
	{
		switch (usage)
		{
			case TextureUsage::Sampled: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			case TextureUsage::Storage: return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			case TextureUsage::RenderTargetColor: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			case TextureUsage::RenderTargetDepth: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			default: COLUMBUS_ASSERT(false);
		}
	}

	static VkSamplerAddressMode TextureAddressModeToVk(TextureAddressMode mode)
	{
		switch (mode)
		{
			case TextureAddressMode::Repeat:         return VK_SAMPLER_ADDRESS_MODE_REPEAT;
			case TextureAddressMode::MirroredRepeat: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
			case TextureAddressMode::ClampToEdge:    return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			default: COLUMBUS_ASSERT(false);
		}
	}

	static VkFilter TextureFilterToVk(TextureFilter2 filter)
	{
		switch (filter)
		{
			case TextureFilter2::Nearest: return VK_FILTER_NEAREST;
			case TextureFilter2::Linear:  return VK_FILTER_LINEAR;
			default: COLUMBUS_ASSERT(false);
		}
	}

	static VkSamplerMipmapMode TextureFilterMipToVk(TextureFilter2 filter)
	{
		switch (filter)
		{
			case TextureFilter2::Nearest: return VK_SAMPLER_MIPMAP_MODE_NEAREST;
			case TextureFilter2::Linear:  return VK_SAMPLER_MIPMAP_MODE_LINEAR;
			default: COLUMBUS_ASSERT(false);
		}
	}

	static VkSampleCountFlagBits SampleCountToVk(uint32 samples)
	{
		switch (samples)
		{
			case 1:  return VK_SAMPLE_COUNT_1_BIT;
			case 2:  return VK_SAMPLE_COUNT_2_BIT;
			case 4:  return VK_SAMPLE_COUNT_4_BIT;
			case 8:  return VK_SAMPLE_COUNT_8_BIT;
			case 16: return VK_SAMPLE_COUNT_16_BIT;
			case 32: return VK_SAMPLE_COUNT_32_BIT;
			case 64: return VK_SAMPLE_COUNT_64_BIT;
			default: COLUMBUS_ASSERT(false);
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
			default: COLUMBUS_ASSERT(false);
		}
	}

	static VkShaderStageFlagBits ShaderTypeToVk(ShaderType type)
	{
		int64_t result = 0;

		if ((type & ShaderType::Vertex) != 0)   result |= VK_SHADER_STAGE_VERTEX_BIT;
		if ((type & ShaderType::Pixel) != 0)    result |= VK_SHADER_STAGE_FRAGMENT_BIT;
		// if ((type & ShaderType::Hull)   != 0)   COLUMBUS_ASSERT_MESSAGE(false, "Tesselation isn't supported"); // TODO:
		// if ((type & ShaderType::Domain) != 0)   COLUMBUS_ASSERT_MESSAGE(false, "Tesselation isn't supported");
		if ((type & ShaderType::Hull)   != 0)   assert(false && "Tesselation isn't supported");
		if ((type & ShaderType::Domain) != 0)   assert(false && "Tesselation isn't supported");
		if ((type & ShaderType::Geometry) != 0) result |= VK_SHADER_STAGE_GEOMETRY_BIT;

		if ((type & ShaderType::Compute) != 0) result |= VK_SHADER_STAGE_COMPUTE_BIT;

		if ((type & ShaderType::Raygen) != 0)       result |= VK_SHADER_STAGE_RAYGEN_BIT_KHR;
		if ((type & ShaderType::Miss) != 0)         result |= VK_SHADER_STAGE_MISS_BIT_KHR;
		if ((type & ShaderType::Anyhit) != 0)       result |= VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
		if ((type & ShaderType::ClosestHit) != 0)   result |= VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
		if ((type & ShaderType::Intersection) != 0) result |= VK_SHADER_STAGE_INTERSECTION_BIT_KHR;

		return static_cast<VkShaderStageFlagBits>(result);
	}

	static VkAttachmentLoadOp AttachmentLoadOpToVk(AttachmentLoadOp op)
	{
		switch (op)
		{
			case AttachmentLoadOp::Load:     return VK_ATTACHMENT_LOAD_OP_LOAD;
			case AttachmentLoadOp::Clear:    return VK_ATTACHMENT_LOAD_OP_CLEAR;
			case AttachmentLoadOp::DontCare: return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			default: COLUMBUS_ASSERT(false);
		}
	}

	static VkBlendFactor BlendToVkBlendFactor(Blend blend)
	{
		switch (blend)
		{
		case Blend::Zero:         return VK_BLEND_FACTOR_ZERO;
		case Blend::One:          return VK_BLEND_FACTOR_ONE;
		case Blend::SrcColor:     return VK_BLEND_FACTOR_SRC_COLOR;
		case Blend::InvSrcColor:  return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
		case Blend::SrcAlpha:     return VK_BLEND_FACTOR_SRC_ALPHA;
		case Blend::InvSrcAlpha:  return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		case Blend::DestAlpha:    return VK_BLEND_FACTOR_DST_ALPHA;
		case Blend::InvDestAlpha: return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
		case Blend::DestColor:    return VK_BLEND_FACTOR_DST_COLOR;
		case Blend::InvDestColor: return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
		case Blend::SrcAlphaSat:  return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;

		// wtf is this
		case Blend::BlendFactor:
		case Blend::InvBlendFactor:
			COLUMBUS_ASSERT(false);

		case Blend::Src1Color:    return  VK_BLEND_FACTOR_SRC1_COLOR;
		case Blend::InvSrc1Color: return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
		case Blend::Src1Alpha:    return VK_BLEND_FACTOR_SRC1_ALPHA;
		case Blend::InvSrc1Alpha: return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
		default: COLUMBUS_ASSERT(false);
		}
	}

	static VkBlendOp BlendOpToVk(BlendOp op)
	{
		switch (op)
		{
		case BlendOp::Add:         return VK_BLEND_OP_ADD;
		case BlendOp::Subtract:    return VK_BLEND_OP_SUBTRACT;
		case BlendOp::RevSubtract: return VK_BLEND_OP_REVERSE_SUBTRACT;
		case BlendOp::Min:         return VK_BLEND_OP_MIN;
		case BlendOp::Max:         return VK_BLEND_OP_MAX;
		default: COLUMBUS_ASSERT(false);
        }
	}

	static VkClearValue AttachmentClearValueToVk(const AttachmentClearValue& Value, bool ForDepth)
	{
		VkClearValue Result;
		if (ForDepth)
		{
			Result.depthStencil.depth = Value.Depth;
			Result.depthStencil.stencil = Value.Stencil;
		}
		else
		{
			memcpy(Result.color.float32, &Value.Color, sizeof(Value.Color));
		}

		return Result;
	}

	static VkQueryType QueryPoolTypeToVk(QueryPoolType Type)
	{
		switch (Type)
		{
			case QueryPoolType::Timestamp: return VK_QUERY_TYPE_TIMESTAMP;
			default: COLUMBUS_ASSERT(false);
		}
	}

}
