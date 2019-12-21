#include <Graphics/OpenGL/DeviceOpenGL.h>
#include <Graphics/OpenGL/BufferOpenGL.h>
#include <Graphics/OpenGL/ShaderOpenGL.h>
#include <Graphics/OpenGL/TextureOpenGL.h>
#include <Graphics/OpenGL/MeshOpenGL.h>
#include <Graphics/OpenGL/FramebufferOpenGL.h>
#include <Graphics/OpenGL/TypeConversions.h>

namespace Columbus
{

	DeviceOpenGL::DeviceOpenGL()
	{

	}

	ShaderProgram* DeviceOpenGL::CreateShaderProgram() const
	{
		return new ShaderProgramOpenGL();
	}

	Texture* DeviceOpenGL::CreateTexture() const
	{
		return new TextureOpenGL();
	}

	Mesh* DeviceOpenGL::CreateMesh() const
	{
		return new MeshOpenGL();
	}

	Framebuffer* DeviceOpenGL::CreateFramebuffer() const
	{
		return new FramebufferOpenGL();
	}

	void DeviceOpenGL::IASetPrimitiveTopology(PrimitiveTopology Topology)
	{
		_currentTopology = Topology;
	}

	void DeviceOpenGL::IASetInputLayout(InputLayout* Layout)
	{
		_currentLayout = *Layout;
	}

	void DeviceOpenGL::IASetVertexBuffers(uint32 StartSlot, uint32 NumBuffers, Buffer** ppBuffers)
	{
		for (int i = 0; i < NumBuffers; i++)
		{
			auto slot = _currentLayout.Elements[i].Slot;
			auto comps = _currentLayout.Elements[i].Components;
			glBindBuffer(GL_ARRAY_BUFFER, *static_cast<GLuint*>(ppBuffers[i]->GetHandle()));
			glVertexAttribPointer(slot, comps, GL_FLOAT, GL_FALSE, 0, (void*)(uintptr_t)0);
			glEnableVertexAttribArray(slot);
		}
	}

	void DeviceOpenGL::OMSetBlendState(BlendState* pBlendState, const float BlendFactor[4], uint32 SampleMask)
	{
		
	}

	void DeviceOpenGL::OMSetDepthStencilState(DepthStencilState* pDepthStencilState, uint32 StencilRef)
	{
		if (pDepthStencilState->Desc.DepthEnable)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);

		glDepthMask(pDepthStencilState->Desc.DepthWriteMask ? GL_TRUE : GL_FALSE);
		glDepthFunc(ComparisonFuncToGL(pDepthStencilState->Desc.DepthFunc));
	}

	bool DeviceOpenGL::CreateBlendState(const BlendStateDesc& Desc, BlendState** ppBlendState)
	{
		*ppBlendState = new BlendState();
		(*ppBlendState)->Desc = Desc;
		return true;
	}

	bool DeviceOpenGL::CreateDepthStencilState(const DepthStencilStateDesc& Desc, DepthStencilState** ppDepthStencilState)
	{
		*ppDepthStencilState = new DepthStencilState();
		(*ppDepthStencilState)->Desc = Desc;
		return true;
	}

	bool DeviceOpenGL::CreateBuffer(const BufferDesc& Desc, Buffer** ppBuffer)
	{
		*ppBuffer = new BufferOpenGL2(Desc);
		auto pBuffer = *ppBuffer;
		auto type = BufferTypeToGL(Desc.BindFlags);
		auto phandle = static_cast<GLuint*>(pBuffer->GetHandle());
		auto cpuAccess = BufferUsageAndAccessToGL(Desc.Usage, Desc.CpuAccess);

		glCreateBuffers(1, static_cast<GLuint*>(pBuffer->GetHandle()));
		glBindBuffer(type, *phandle);
		glBufferData(type, Desc.Size, nullptr, cpuAccess);
		glBindBuffer(type, 0);
		return true;
	}

	void DeviceOpenGL::BindBufferRange(Buffer* pBuffer, uint32 Index, uint32 Offset, uint32 Size)
	{
		auto type = BufferTypeToGL(pBuffer->GetDesc().BindFlags);
		auto glhandle = *static_cast<GLuint*>(pBuffer->GetHandle());
		glBindBufferRange(type, Index, glhandle, Offset, Size);
	}

	void DeviceOpenGL::MapBuffer(Buffer* pBuffer, BufferMapAccess MapAccess, void*& MappedData)
	{
		uint32 A = 0;
		switch (MapAccess)
		{
		case BufferMapAccess::Read:      A = GL_READ_ONLY;  break;
		case BufferMapAccess::Write:     A = GL_WRITE_ONLY; break;
		case BufferMapAccess::ReadWrite: A = GL_READ_WRITE; break;
		}

		auto type = BufferTypeToGL(pBuffer->GetDesc().BindFlags);
		auto glhandle = *static_cast<GLuint*>(pBuffer->GetHandle());

		glBindBuffer(type, glhandle);
		MappedData = glMapBuffer(type, A);
	}

	void DeviceOpenGL::UnmapBuffer(Buffer* pBuffer)
	{
		auto type = BufferTypeToGL(pBuffer->GetDesc().BindFlags);
		auto glhandle = *static_cast<GLuint*>(pBuffer->GetHandle());
		glBindBuffer(type, glhandle);
		glUnmapBuffer(type);
	}

	void DeviceOpenGL::Draw(uint32 VertexCount, uint32 StartVertexLocation)
	{
		auto mode = PrimitiveTopologyToGL(_currentTopology);
		glDrawArrays(mode, StartVertexLocation, VertexCount);
	}
	
	DeviceOpenGL::~DeviceOpenGL()
	{

	}

}


