#include <Graphics/OpenGL/DeviceOpenGL.h>
#include <Graphics/OpenGL/BufferOpenGL.h>
#include <Graphics/OpenGL/ShaderOpenGL.h>
#include <Graphics/OpenGL/TextureOpenGL.h>
#include <Graphics/OpenGL/MeshOpenGL.h>
#include <Graphics/OpenGL/FramebufferOpenGL.h>
#include <Graphics/OpenGL/TypeConversions.h>
#include <RenderAPIOpenGL/OpenGL.h>

namespace Columbus
{

	DeviceOpenGL::DeviceOpenGL()
	{
		_currentApi = GraphicsAPI::OpenGL;
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

	static GLuint __vao = 0;
	void DeviceOpenGL::IASetVertexBuffers(uint32 StartSlot, uint32 NumBuffers, Buffer** ppBuffers)
	{
		if (__vao == 0) glGenVertexArrays(1, &__vao);

		glBindVertexArray(__vao);
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
		const auto& Desc = pBlendState->GetDesc();

		if (Desc.AlphaToCoverageEnable)
		{
			glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
		}
		else
		{
			glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
		}

		bool bBlendEnable = false;
		for (int i = 0; i < Desc.NumRenderTargets; i++)
		{
			if (Desc.RenderTarget[i].BlendEnable)
				bBlendEnable = true;
		}

		if (bBlendEnable)
			glEnable(GL_BLEND);
		else
			glDisable(GL_BLEND);

		if (Desc.IndependentBlendEnable)
		{
			for (int i = 0; i < Desc.NumRenderTargets; i++)
			{
				const auto& RTi = Desc.RenderTarget[i];
				if (RTi.BlendEnable)
				{
					glEnablei(GL_BLEND, i);
					glBlendFuncSeparatei(i, BlendToGL(RTi.SrcBlend), BlendToGL(RTi.DestBlend), BlendToGL(RTi.SrcBlendAlpha), BlendToGL(RTi.DestBlendAlpha));
					glBlendEquationSeparatei(i, BlendOpToGL(RTi.Op), BlendOpToGL(RTi.OpAlpha));
				}
				else
				{
					glDisablei(GL_BLEND, i);
				}
			}
		}
		else
		{
			const auto& RT0 = Desc.RenderTarget[0];
			if (RT0.BlendEnable)
			{
				glBlendFuncSeparate(BlendToGL(RT0.SrcBlend), BlendToGL(RT0.DestBlend), BlendToGL(RT0.SrcBlendAlpha), BlendToGL(RT0.DestBlendAlpha));
				glBlendEquationSeparate(BlendOpToGL(RT0.Op), BlendOpToGL(RT0.OpAlpha));
			}
			else
			{
				glDisable(GL_BLEND);
			}
		}
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

	void DeviceOpenGL::RSSetState(RasterizerState* pRasterizerState)
	{
		const auto& desc = pRasterizerState->GetDesc();
		switch (desc.Cull)
		{
		case CullMode::No:
			glDisable(GL_CULL_FACE);
			break;
		case CullMode::Front:
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);
			glFrontFace(desc.FrontCounterClockwise ? GL_CCW : GL_CW);
			break;
		case CullMode::Back:
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			glFrontFace(desc.FrontCounterClockwise ? GL_CCW : GL_CW);
			break;
		}

		switch (desc.Fill)
		{
		case FillMode::Wireframe:
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			break;
		case FillMode::Solid:
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			break;
		}
	}

	void DeviceOpenGL::SetShader(ShaderProgram* Prog)
	{
		if (Prog != nullptr)
		{
			if (!Prog->IsError())
			{
				if (!Prog->IsCompiled())
				{
					Prog->Compile();
				}
			}

			static_cast<ShaderProgramOpenGL*>(Prog)->Bind();
		}
		else
		{
			glUseProgram(0);
		}
	}

	void DeviceOpenGL::SetComputePipelineState(ComputePipelineState* State)
	{
		glUseProgram(State->progid);
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

	bool DeviceOpenGL::CreateRasterizerState(const RasterizerStateDesc& Desc, RasterizerState** ppRasterizerState)
	{
		*ppRasterizerState = new RasterizerState();
		(*ppRasterizerState)->Desc = Desc;
		return true;
	}

	bool DeviceOpenGL::CreateBuffer(const BufferDesc& Desc, SubresourceData* pInitialData, Buffer** ppBuffer)
	{
		*ppBuffer = new BufferOpenGL(Desc);
		auto pBuffer = *ppBuffer;
		auto type = BufferTypeToGL(Desc.BindFlags);
		auto phandle = static_cast<GLuint*>(pBuffer->GetHandle());
		auto cpuAccess = BufferUsageAndAccessToGL(Desc.Usage, Desc.CpuAccess);

		const void* initialData = pInitialData != nullptr ? pInitialData->pSysMem : nullptr;

		glCreateBuffers(1, static_cast<GLuint*>(pBuffer->GetHandle()));
		glBindBuffer(type, *phandle);
		if (OpenGL::SupportsBufferStorage())
			glBufferStorage(type, Desc.Size, initialData, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_DYNAMIC_STORAGE_BIT);
		else
			glBufferData(type, Desc.Size, initialData, cpuAccess);
		glBindBuffer(type, 0);
		return true;
	}

	void DeviceOpenGL::BindBufferBase(Buffer* pBuffer, uint32 Index)
	{
		auto type = BufferTypeToGL(pBuffer->GetDesc().BindFlags);
		auto glhandle = *static_cast<GLuint*>(pBuffer->GetHandle());
		glBindBufferBase(type, Index, glhandle);
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

	bool DeviceOpenGL::CreateComputePipelineState(const ComputePipelineStateDesc& Desc, ComputePipelineState** ppComputePipelineState)
	{
		*ppComputePipelineState = new ComputePipelineState(Desc);
		auto pComputePipelineState = *ppComputePipelineState;

		GLint len = Desc.CS.length();
		const GLchar* str = Desc.CS.c_str();

		pComputePipelineState->shadid = glCreateShader(GL_COMPUTE_SHADER);
		glShaderSource(pComputePipelineState->shadid, 1, &str, &len);
		glCompileShader(pComputePipelineState->shadid);

		int32 Status = GL_TRUE;
		int32 Length = 0;
		char* Error = nullptr;

		glGetShaderiv(pComputePipelineState->shadid, GL_COMPILE_STATUS, &Status);
		glGetShaderiv(pComputePipelineState->shadid, GL_INFO_LOG_LENGTH, &Length);
		Error = new char[Length];
		glGetShaderInfoLog(pComputePipelineState->shadid, Length, &Length, Error);

		if (Status == GL_FALSE)
		{
			Log::Error("%s shader (%s): %s", "Compute", "IDK", Error);
		}
		else if (Length > 1)
		{
			Log::Warning("%s shader (%s): %s", "Compute", "IDK", Error);
		}

		pComputePipelineState->progid = glCreateProgram();
		glAttachShader(pComputePipelineState->progid, pComputePipelineState->shadid);
		glLinkProgram(pComputePipelineState->progid);
		return true;
	}

	void DeviceOpenGL::Dispatch(uint32 X, uint32 Y, uint32 Z)
	{
		glDispatchCompute(X, Y, Z);
	}

	void DeviceOpenGL::Draw(uint32 VertexCount, uint32 StartVertexLocation)
	{
		auto mode = PrimitiveTopologyToGL(_currentTopology);
		glDrawArrays(mode, StartVertexLocation, VertexCount);
	}

	void DeviceOpenGL::BeginMarker(const char* Str)
	{
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, strlen(Str), Str);
	}

	void DeviceOpenGL::EndMarker()
	{
		glPopDebugGroup();
	}
	
	DeviceOpenGL::~DeviceOpenGL()
	{

	}

}
