#pragma once

#include <Graphics/Device.h>

namespace Columbus
{

	class DeviceOpenGL : public Device
	{
	public:
		DeviceOpenGL();

		virtual ShaderProgram* CreateShaderProgram() const override;
		virtual Texture* CreateTexture() const override;
		virtual Mesh* CreateMesh() const override;
		virtual Framebuffer* CreateFramebuffer() const override;

		virtual void IASetVertexBuffers(uint32 StartSlot, uint32 NumBuffers, Buffer** ppBuffers) final override;

		virtual bool CreateBuffer(const BufferDesc& Desc, Buffer** ppBuffer) final override;
		virtual void BindBufferRange(Buffer* pBuffer, uint32 Index, uint32 Offset, uint32 Size) final override;
		virtual void MapBuffer(Buffer* pBuffer, BufferMapAccess MapAccess, void*& MappedData) final override;
		virtual void UnmapBuffer(Buffer* pBuffer) final override;

		virtual void Draw(uint32 VertexCount, uint32 StartVertexLocation) final override;

		virtual ~DeviceOpenGL() override;
	};

}


