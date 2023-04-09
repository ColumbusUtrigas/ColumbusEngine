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

		// LEGACY
		virtual ShaderProgram* CreateShaderProgram() const override;
		virtual Texture* CreateTexture() const override;
		virtual Mesh* CreateMesh() const override;
		virtual Framebuffer* CreateFramebuffer() const override;
		virtual void IASetInputLayout(InputLayout* Layout) final override;
		virtual void OMSetBlendState(BlendState* pBlendState, const float BlendFactor[4], uint32 SampleMask) final override;
		virtual void OMSetDepthStencilState(DepthStencilState* pDepthStencilState, uint32 StencilRef) final override;
		virtual void RSSetState(RasterizerState* pRasterizerState) final override;
		virtual void SetShader(ShaderProgram* Prog) final override;
		virtual bool CreateBlendState(const BlendStateDesc& Desc, BlendState** ppBlendState) final override;
		virtual bool CreateDepthStencilState(const DepthStencilStateDesc& Desc, DepthStencilState** ppDepthStencilState) final override;
		virtual bool CreateRasterizerState(const RasterizerStateDesc& Desc, RasterizerState** ppRasterizerState) final override;
		virtual void BindBufferBase(Buffer* pBuffer, uint32 Index) final override;
		virtual void BindBufferRange(Buffer* pBuffer, uint32 Index, uint32 Offset, uint32 Size) final override;
		//////////////////////////////////////////////

		void IASetVertexBuffers(uint32 StartSlot, uint32 NumBuffers, Buffer** ppBuffers) final override;
		void IASetIndexBuffer(Buffer* pIndexBuffer, IndexFormat Format, uint32 Offset) final override;

		void RSSetViewports(uint32 Num, Viewport* pViewports) final override;
		void RSSetScissorRects(uint32 Num, ScissorRect* pScissors) final override;

		void SetGraphicsPipeline(Graphics::GraphicsPipeline* pPipeline) final override;
		void SetGraphicsCBV(uint32 slot, Buffer* pBuf) final override;

		void SetComputePipeline(ComputePipeline* pPipeline) override;

		bool CreateBuffer(const BufferDesc& Desc, SubresourceData* pInitialData, Buffer** ppBuffer) final override;
		void MapBuffer(Buffer* pBuffer, BufferMapAccess MapAccess, void*& MappedData) final override;
		void UnmapBuffer(Buffer* pBuffer) final override;

		bool CreateGraphicsPipeline(const Graphics::GraphicsPipelineDesc& Desc, Graphics::GraphicsPipeline** ppGraphicsPipeline) final override;
		bool CreateComputePipeline(const ComputePipelineDesc& desc, ComputePipeline** ppComputePipelineState) final override;

		void Dispatch(uint32 X, uint32 Y, uint32 Z) final override;
		void Draw(uint32 VertexCount, uint32 StartVertexLocation) final override;
		void DrawIndexed(uint32 IndexCount, uint32 StartIndexLocation, int BaseVertexLocation) final override;

		void BeginMarker(const char* Str) final override;
		void EndMarker() final override;

		~DeviceOpenGL() override;

		//TEST
		void SetTextures(Texture** ppTextures, int num);
	};

}
