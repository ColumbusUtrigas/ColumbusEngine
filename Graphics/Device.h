#pragma once

#include <Graphics/Shader.h>
#include <Graphics/Texture.h>
#include <Graphics/Mesh.h>
#include <Graphics/Framebuffer.h>
#include <Graphics/Buffer.h>

#include <GL/glew.h>

namespace Columbus
{

	enum class PrimitiveTopology
	{
		TriangleList
	};

	struct InputLayoutElement
	{
		uint32 Slot;
		uint32 Components; //float!!!
	};

	struct InputLayout
	{
		InputLayoutElement Elements[16];
		uint32 NumElements = 0;
	};

	class Device
	{
	private:
		DefaultShaders* gDefaultShaders = nullptr;
		DefaultTextures* gDefaultTextures = nullptr;
	protected:
		InputLayout _currentLayout;
		PrimitiveTopology _currentTopology;
	public:
		static GLenum _GetBufferTypeGL(BufferType type)
		{
			switch (type)
			{
			case BufferType::Array: return GL_ARRAY_BUFFER;
			case BufferType::Index: return GL_ELEMENT_ARRAY_BUFFER;
			case BufferType::Uniform: return GL_UNIFORM_BUFFER;
			default: return 0;
			}
		}

		static GLenum _GetBufferCpuAccessGL(BufferUsage Usage, BufferCpuAccess CpuAccess)
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

		static GLenum _GetPrimitiveTopologyGL(PrimitiveTopology topology)
		{
			switch (topology)
			{
			case PrimitiveTopology::TriangleList: return GL_TRIANGLES;
			}
		}

		Device();

		void Initialize();
		void Shutdown();

		virtual void IASetPrimitiveTopology(PrimitiveTopology Topology)
		{
			_currentTopology = Topology;
		}
		virtual void IASetInputLayout(InputLayout* Layout)
		{
			_currentLayout = *Layout;
		}

		virtual void IASetVertexBuffers(uint32 StartSlot, uint32 NumBuffers, Buffer** ppBuffers) = 0;

		virtual bool CreateBuffer(const BufferDesc& Desc, Buffer** ppBuffer) = 0;
		virtual void BindBufferRange(Buffer* pBuffer, uint32 Index, uint32 Offset, uint32 Size) = 0;
		virtual void MapBuffer(Buffer* pBuffer, BufferMapAccess MapAccess, void*& MappedData) = 0;
		virtual void UnmapBuffer(Buffer* pBuffer) = 0;

		virtual void Draw(uint32 VertexCount, uint32 StartVertexLocation) = 0;

		virtual ShaderProgram* CreateShaderProgram() const;
		virtual Texture* CreateTexture() const;
		virtual Mesh* CreateMesh() const;
		virtual Framebuffer* CreateFramebuffer() const;

		DefaultShaders* GetDefaultShaders() { return gDefaultShaders; }
		DefaultTextures* GetDefaultTextures()  { return gDefaultTextures; };

		virtual ~Device();
	};

	extern Device* gDevice;

}


