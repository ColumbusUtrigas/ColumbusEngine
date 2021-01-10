#pragma once

#include <Graphics/Types.h>
#include <GL/glew.h>

namespace Columbus
{

	static GLenum BufferTypeToGL(BufferType type)
	{
		switch (type)
		{
		case BufferType::Array:   return GL_ARRAY_BUFFER;
		case BufferType::Index:   return GL_ELEMENT_ARRAY_BUFFER;
		case BufferType::Uniform: return GL_UNIFORM_BUFFER;
		case BufferType::UAV:     return GL_SHADER_STORAGE_BUFFER;
		}
	}

	static GLenum BufferUsageAndAccessToGL(BufferUsage Usage, BufferCpuAccess CpuAccess)
	{
		switch (CpuAccess)
		{
		case BufferCpuAccess::Write:
		{
			switch (Usage)
			{
			case BufferUsage::Static:  return GL_STATIC_DRAW;  break;
			case BufferUsage::Dynamic: return GL_DYNAMIC_DRAW; break;
			case BufferUsage::Stream:  return GL_STREAM_DRAW;  break;
			}

			break;
		}

		case BufferCpuAccess::Read:
		{
			switch (Usage)
			{
			case BufferUsage::Static:  return GL_STATIC_READ;  break;
			case BufferUsage::Dynamic: return GL_DYNAMIC_READ; break;
			case BufferUsage::Stream:  return GL_STREAM_READ;  break;
			}

			break;
		}

		case BufferCpuAccess::Copy:
		{
			switch (Usage)
			{
			case BufferUsage::Static:  return GL_STATIC_COPY;  break;
			case BufferUsage::Dynamic: return GL_DYNAMIC_COPY; break;
			case BufferUsage::Stream:  return GL_STREAM_COPY;  break;
			}

			break;
		}
		}

		return 0;
	}

	static std::pair<GLenum, int> PrimitiveTopologyToGL(PrimitiveTopology topology)
	{
		switch (topology)
		{
		case PrimitiveTopology::Undefined:    return std::make_pair(GL_NONE, 0);
		case PrimitiveTopology::PointList:    return std::make_pair(GL_POINTS, 0);
		case PrimitiveTopology::LineList:     return std::make_pair(GL_LINES, 0);
		case PrimitiveTopology::TriangleList: return std::make_pair(GL_TRIANGLES, 0);
		case PrimitiveTopology::PatchList_1:  return std::make_pair(GL_PATCHES, 1);
		case PrimitiveTopology::PatchList_2:  return std::make_pair(GL_PATCHES, 2);
		case PrimitiveTopology::PatchList_3:  return std::make_pair(GL_PATCHES, 3);
		case PrimitiveTopology::PatchList_4:  return std::make_pair(GL_PATCHES, 4);
		case PrimitiveTopology::PatchList_5:  return std::make_pair(GL_PATCHES, 5);
		case PrimitiveTopology::PatchList_6:  return std::make_pair(GL_PATCHES, 6);
		case PrimitiveTopology::PatchList_7:  return std::make_pair(GL_PATCHES, 7);
		case PrimitiveTopology::PatchList_8:  return std::make_pair(GL_PATCHES, 8);
		case PrimitiveTopology::PatchList_9:  return std::make_pair(GL_PATCHES, 9);
		case PrimitiveTopology::PatchList_10: return std::make_pair(GL_PATCHES, 10);
		case PrimitiveTopology::PatchList_11: return std::make_pair(GL_PATCHES, 11);
		case PrimitiveTopology::PatchList_12: return std::make_pair(GL_PATCHES, 12);
		case PrimitiveTopology::PatchList_13: return std::make_pair(GL_PATCHES, 13);
		case PrimitiveTopology::PatchList_14: return std::make_pair(GL_PATCHES, 14);
		case PrimitiveTopology::PatchList_15: return std::make_pair(GL_PATCHES, 15);
		case PrimitiveTopology::PatchList_16: return std::make_pair(GL_PATCHES, 16);
		case PrimitiveTopology::PatchList_17: return std::make_pair(GL_PATCHES, 17);
		case PrimitiveTopology::PatchList_18: return std::make_pair(GL_PATCHES, 18);
		case PrimitiveTopology::PatchList_19: return std::make_pair(GL_PATCHES, 19);
		case PrimitiveTopology::PatchList_20: return std::make_pair(GL_PATCHES, 20);
		case PrimitiveTopology::PatchList_21: return std::make_pair(GL_PATCHES, 21);
		case PrimitiveTopology::PatchList_22: return std::make_pair(GL_PATCHES, 22);
		case PrimitiveTopology::PatchList_23: return std::make_pair(GL_PATCHES, 23);
		case PrimitiveTopology::PatchList_24: return std::make_pair(GL_PATCHES, 24);
		case PrimitiveTopology::PatchList_25: return std::make_pair(GL_PATCHES, 25);
		case PrimitiveTopology::PatchList_26: return std::make_pair(GL_PATCHES, 26);
		case PrimitiveTopology::PatchList_27: return std::make_pair(GL_PATCHES, 27);
		case PrimitiveTopology::PatchList_28: return std::make_pair(GL_PATCHES, 28);
		case PrimitiveTopology::PatchList_29: return std::make_pair(GL_PATCHES, 29);
		case PrimitiveTopology::PatchList_30: return std::make_pair(GL_PATCHES, 30);
		case PrimitiveTopology::PatchList_31: return std::make_pair(GL_PATCHES, 31);
		case PrimitiveTopology::PatchList_32: return std::make_pair(GL_PATCHES, 32);
		}
	}

	static GLenum IndexFormatToGL(IndexFormat format)
	{
		switch (format)
		{
		case IndexFormat::Uint16: return GL_UNSIGNED_SHORT;
		case IndexFormat::Uint32: return GL_UNSIGNED_INT;
		}
	}

	static GLenum ComparisonFuncToGL(ComparisonFunc func)
	{
		switch (func)
		{
		case ComparisonFunc::Less:     return GL_LESS;
		case ComparisonFunc::Greater:  return GL_GREATER;
		case ComparisonFunc::LEqual:   return GL_LEQUAL;
		case ComparisonFunc::GEqual:   return GL_GEQUAL;
		case ComparisonFunc::Equal:    return GL_EQUAL;
		case ComparisonFunc::NotEqual: return GL_NOTEQUAL;
		case ComparisonFunc::Never:    return GL_NEVER;
		case ComparisonFunc::Always:   return GL_ALWAYS;
		}
	}
	
	static GLenum StencilOpToGL(StencilOp op)
	{
		switch (op)
		{
		case StencilOp::Keep:    return GL_KEEP;
		case StencilOp::Zero:    return GL_ZERO;
		case StencilOp::Replace: return GL_REPLACE;
		case StencilOp::IncrSat: return GL_INCR;
		case StencilOp::DecrSat: return GL_DECR;
		case StencilOp::Invert:  return GL_INVERT;
		case StencilOp::Incr:    return GL_INCR_WRAP;
		case StencilOp::Decr:    return GL_DECR_WRAP;
		}
	}

	static GLenum BlendToGL(Blend blend)
	{
		switch (blend)
		{
		case Blend::Zero:           return GL_ZERO;
		case Blend::One:            return GL_ONE;
		case Blend::SrcColor:       return GL_SRC_COLOR;
		case Blend::InvSrcColor:    return GL_ONE_MINUS_SRC_COLOR;
		case Blend::SrcAlpha:       return GL_SRC_ALPHA;
		case Blend::InvSrcAlpha:    return GL_ONE_MINUS_SRC_ALPHA;
		case Blend::DestAlpha:      return GL_DST_ALPHA;
		case Blend::InvDestAlpha:   return GL_ONE_MINUS_DST_ALPHA;
		case Blend::DestColor:      return GL_DST_COLOR;
		case Blend::InvDestColor:   return GL_ONE_MINUS_SRC_COLOR;
		case Blend::SrcAlphaSat:    return GL_SRC_ALPHA_SATURATE;
		case Blend::BlendFactor:    return GL_CONSTANT_COLOR;
		case Blend::InvBlendFactor: return GL_ONE_MINUS_CONSTANT_COLOR;
		case Blend::Src1Color:      return GL_SRC1_COLOR;
		case Blend::InvSrc1Color:   return GL_ONE_MINUS_SRC1_COLOR;
		case Blend::Src1Alpha:      return GL_SRC1_ALPHA;
		case Blend::InvSrc1Alpha:   return GL_ONE_MINUS_SRC1_ALPHA;
		}
	}

	static GLenum BlendOpToGL(BlendOp op)
	{
		switch (op)
		{
		case BlendOp::Add:         return GL_FUNC_ADD;
		case BlendOp::Subtract:    return GL_FUNC_SUBTRACT;
		case BlendOp::RevSubtract: return GL_FUNC_REVERSE_SUBTRACT;
		case BlendOp::Min:         return GL_MIN;
		case BlendOp::Max:         return GL_MAX;
		}
	}

}
