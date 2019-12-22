#pragma once

#include <Graphics/Shader.h>
#include <Graphics/ComputePipelineState.h>
#include <Graphics/Texture.h>
#include <Graphics/Mesh.h>
#include <Graphics/Framebuffer.h>
#include <Graphics/Buffer.h>
#include <Graphics/DepthStencilState.h>
#include <Graphics/BlendState.h>
#include <Graphics/RasterizerState.h>
#include <Graphics/InputLayout.h>
#include <Graphics/Types.h>

namespace Columbus
{

	class Device
	{
	private:
		DefaultShaders* gDefaultShaders = nullptr;
		DefaultTextures* gDefaultTextures = nullptr;
	public:
		Device();

		void Initialize();
		void Shutdown();

		virtual void IASetPrimitiveTopology(PrimitiveTopology Topology) = 0;
		virtual void IASetInputLayout(InputLayout* Layout) = 0;
		virtual void IASetVertexBuffers(uint32 StartSlot, uint32 NumBuffers, Buffer** ppBuffers) = 0;

		virtual void OMSetBlendState(BlendState* pBlendState, const float BlendFactor[4], uint32 SampleMask) = 0;
		virtual void OMSetDepthStencilState(DepthStencilState* pDepthStencilState, uint32 StencilRef) = 0;

		virtual void RSSetState(RasterizerState* pRasterizerState) = 0;
		//virtual void RSSetViewports() = 0;
		//virtual void RSSetScissorRects() = 0;

		virtual void SetShader(ShaderProgram* Prog) = 0;

		virtual bool CreateBlendState(const BlendStateDesc& Desc, BlendState** ppBlendState) = 0;
		virtual bool CreateDepthStencilState(const DepthStencilStateDesc& Desc, DepthStencilState** ppDepthStencilState) = 0;
		virtual bool CreateRasterizerState(const RasterizerStateDesc& Desc, RasterizerState** ppRasterizerState) = 0;

		virtual bool CreateBuffer(const BufferDesc& Desc, SubresourceData* pInitialData, Buffer** ppBuffer) = 0;
		virtual void BindBufferRange(Buffer* pBuffer, uint32 Index, uint32 Offset, uint32 Size) = 0;
		virtual void MapBuffer(Buffer* pBuffer, BufferMapAccess MapAccess, void*& MappedData) = 0;
		virtual void UnmapBuffer(Buffer* pBuffer) = 0;

		//virtual bool CreateComputePipelineState(const ComputePipelineStateDesc& Desc, ComputePipelineState** ppComputePipelineState) = 0;

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


