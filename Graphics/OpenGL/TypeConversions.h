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
		}
	}

	static GLenum BufferUsageAndAccessToGL(BufferUsage Usage, BufferCpuAccess CpuAccess)
	{
		switch (Usage)
		{
		case BufferUsage::Write:
		{
			switch (CpuAccess)
			{
			case BufferCpuAccess::Static:  return GL_STATIC_DRAW;  break;
			case BufferCpuAccess::Dynamic: return GL_DYNAMIC_DRAW; break;
			case BufferCpuAccess::Stream:  return GL_STREAM_DRAW;  break;
			}

			break;
		}

		case BufferUsage::Read:
		{
			switch (CpuAccess)
			{
			case BufferCpuAccess::Static:  return GL_STATIC_READ;  break;
			case BufferCpuAccess::Dynamic: return GL_DYNAMIC_READ; break;
			case BufferCpuAccess::Stream:  return GL_STREAM_READ;  break;
			}

			break;
		}

		case BufferUsage::Copy:
		{
			switch (CpuAccess)
			{
			case BufferCpuAccess::Static:  return GL_STATIC_COPY;  break;
			case BufferCpuAccess::Dynamic: return GL_DYNAMIC_COPY; break;
			case BufferCpuAccess::Stream:  return GL_STREAM_COPY;  break;
			}

			break;
		}
		}

		return 0;
	}

	static GLenum PrimitiveTopologyToGL(PrimitiveTopology topology)
	{
		switch (topology)
		{
		case PrimitiveTopology::TriangleList: return GL_TRIANGLES;
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

}
