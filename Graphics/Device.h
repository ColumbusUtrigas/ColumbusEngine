#pragma once

#include <Graphics/Shader.h>
#include <Graphics/GraphicsPipeline.h>
#include <Graphics/ComputePipeline.h>
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
	protected:
		GraphicsAPI _currentApi = GraphicsAPI::None;
	public:
		Device();

		GraphicsAPI GetCurrentAPI();

		void Initialize();
		void Shutdown();

		virtual void IASetInputLayout(InputLayout* Layout) = 0;
		virtual void IASetVertexBuffers(uint32 StartSlot, uint32 NumBuffers, Buffer** ppBuffers) = 0;
		virtual void IASetIndexBuffer(Buffer* pIndexBuffer, IndexFormat Format, uint32 Offset) = 0;

		virtual void OMSetBlendState(BlendState* pBlendState, const float BlendFactor[4], uint32 SampleMask) = 0;
		virtual void OMSetDepthStencilState(DepthStencilState* pDepthStencilState, uint32 StencilRef) = 0;

		virtual void RSSetState(RasterizerState* pRasterizerState) = 0;
		virtual void RSSetViewports(uint32 Num, Viewport* pViewports) = 0;
		virtual void RSSetScissorRects(uint32 Num, ScissorRect* pScissors) = 0;

		virtual void SetShader(ShaderProgram* Prog) = 0;

		virtual void SetGraphicsPipeline(Columbus::Graphics::GraphicsPipeline* pPipeline) {}
		virtual void SetComputePipelineState(ComputePipeline* State) = 0;
		//virtual void SetGraphicsPipelineState(GraphicsPipelineState* State) = 0;

		virtual bool CreateBlendState(const BlendStateDesc& Desc, BlendState** ppBlendState) = 0;
		virtual bool CreateDepthStencilState(const DepthStencilStateDesc& Desc, DepthStencilState** ppDepthStencilState) = 0;
		virtual bool CreateRasterizerState(const RasterizerStateDesc& Desc, RasterizerState** ppRasterizerState) = 0;

		virtual bool CreateBuffer(const BufferDesc& Desc, SubresourceData* pInitialData, Buffer** ppBuffer) = 0;
		virtual void BindBufferBase(Buffer* pBuffer, uint32 Index) = 0;
		virtual void BindBufferRange(Buffer* pBuffer, uint32 Index, uint32 Offset, uint32 Size) = 0;
		virtual void MapBuffer(Buffer* pBuffer, BufferMapAccess MapAccess, void*& MappedData) = 0;
		virtual void UnmapBuffer(Buffer* pBuffer) = 0;

		virtual bool CreateGraphicsPipeline(const Columbus::Graphics::GraphicsPipelineDesc& Desc, Columbus::Graphics::GraphicsPipeline** ppPipeline) = 0;
		virtual bool CreateComputePipelineState(const ComputePipelineDesc& Desc, ComputePipeline** ppComputePipelineState) = 0;

		virtual void Dispatch(uint32 X, uint32 Y, uint32 Z) = 0;
		virtual void Draw(uint32 VertexCount, uint32 StartVertexLocation) = 0;
		virtual void DrawIndexed(uint32 IndexCount, uint32 StartIndexLocation, int BaseVertexLocation) = 0;

		virtual void BeginMarker(const char* Str) = 0;
		virtual void EndMarker() = 0;

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


