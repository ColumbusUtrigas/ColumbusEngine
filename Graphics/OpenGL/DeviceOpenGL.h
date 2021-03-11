#pragma once

#include <Graphics/Device.h>

namespace Columbus
{

	class DeviceOpenGL : public Device
	{
	private:
		PrimitiveTopology _currentTopology;
		IndexFormat _currentIndexFormat;
		InputLayout _currentLayout;
		Graphics::GraphicsPipeline* _currentPipeline = nullptr;

		uint32 _currentIndexOffset;
	public:
		DeviceOpenGL();

		virtual ShaderProgram* CreateShaderProgram() const override;
		virtual Texture* CreateTexture() const override;
		virtual Mesh* CreateMesh() const override;
		virtual Framebuffer* CreateFramebuffer() const override;

		virtual void IASetInputLayout(InputLayout* Layout) final override;
		virtual void IASetVertexBuffers(uint32 StartSlot, uint32 NumBuffers, Buffer** ppBuffers) final override;
		virtual void IASetIndexBuffer(Buffer* pIndexBuffer, IndexFormat Format, uint32 Offset) final override;

		virtual void OMSetBlendState(BlendState* pBlendState, const float BlendFactor[4], uint32 SampleMask) final override;
		virtual void OMSetDepthStencilState(DepthStencilState* pDepthStencilState, uint32 StencilRef) final override;

		virtual void RSSetState(RasterizerState* pRasterizerState) final override;
		virtual void RSSetViewports(uint32 Num, Viewport* pViewports) final override;
		virtual void RSSetScissorRects(uint32 Num, ScissorRect* pScissors) final override;

		virtual void SetShader(ShaderProgram* Prog) final override;

		virtual void SetGraphicsPipeline(Columbus::Graphics::GraphicsPipeline* pPipeline) final override;
		virtual void SetGraphicsCBV(uint32 slot, Columbus::Buffer* pBuf) final override;

		virtual void SetComputePipelineState(ComputePipeline* State) final override;

		virtual bool CreateBlendState(const BlendStateDesc& Desc, BlendState** ppBlendState) final override;
		virtual bool CreateDepthStencilState(const DepthStencilStateDesc& Desc, DepthStencilState** ppDepthStencilState) final override;
		virtual bool CreateRasterizerState(const RasterizerStateDesc& Desc, RasterizerState** ppRasterizerState) final override;

		virtual bool CreateBuffer(const BufferDesc& Desc, SubresourceData* pInitialData, Buffer** ppBuffer) final override;
		virtual void BindBufferBase(Buffer* pBuffer, uint32 Index) final override;
		virtual void BindBufferRange(Buffer* pBuffer, uint32 Index, uint32 Offset, uint32 Size) final override;
		virtual void MapBuffer(Buffer* pBuffer, BufferMapAccess MapAccess, void*& MappedData) final override;
		virtual void UnmapBuffer(Buffer* pBuffer) final override;

		virtual bool CreateGraphicsPipeline(const Graphics::GraphicsPipelineDesc& Desc, Graphics::GraphicsPipeline** ppGraphicsPipeline) final override;
		virtual bool CreateComputePipelineState(const ComputePipelineDesc& Desc, ComputePipeline** ppComputePipelineState) final override;

		virtual void Dispatch(uint32 X, uint32 Y, uint32 Z) final override;
		virtual void Draw(uint32 VertexCount, uint32 StartVertexLocation) final override;
		virtual void DrawIndexed(uint32 IndexCount, uint32 StartIndexLocation, int BaseVertexLocation) final override;

		virtual void BeginMarker(const char* Str) final override;
		virtual void EndMarker() final override;

		virtual ~DeviceOpenGL() override;

		//TEST
		void SetTextures(Texture** ppTextures, int num);
	};

}


