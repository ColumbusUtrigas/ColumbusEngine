#pragma once

#include <Graphics/Device.h>
#include <Graphics/GraphicsPipeline.h>
#include <Graphics/ComputePipeline.h>
#include <Graphics/Buffer.h>
#include <Graphics/Types.h>
#include <d3d12.h>

namespace Columbus::Graphics::DX12
{

	class tmpDeviceDX12 : public Device
	{
	private:
		ID3D12Device* _device;
		ID3D12CommandQueue* _queue;
		ID3D12GraphicsCommandList* _cmdList;
	public:
		tmpDeviceDX12(ID3D12Device* device, ID3D12CommandQueue* queue, ID3D12GraphicsCommandList* cmdList) :
			_device(device), _queue(queue), _cmdList(cmdList) {}

		virtual void IASetInputLayout(InputLayout* Layout) {}
		void IASetVertexBuffers(uint32 StartSlot, uint32 NumBuffers, Buffer** ppBuffers) override;
		void IASetIndexBuffer(Buffer* pIndexBuffer, IndexFormat Format, uint32 Offset) override;

		virtual void OMSetBlendState(BlendState* pBlendState, const float BlendFactor[4], uint32 SampleMask) {}
		virtual void OMSetDepthStencilState(DepthStencilState* pDepthStencilState, uint32 StencilRef) {}

		virtual void RSSetState(RasterizerState* pRasterizerState) {}
		void RSSetViewports(uint32 Num, Viewport* pViewports) override;
		void RSSetScissorRects(uint32 Num, ScissorRect* pScissors) override;

		virtual void SetShader(ShaderProgram* Prog) {}

		void SetGraphicsPipeline(GraphicsPipeline* pPipeline) override;
		void SetGraphicsCBV(uint32 slot, Buffer* pBuf) override;

		virtual void SetComputePipelineState(ComputePipeline* State) {}

		virtual bool CreateBlendState(const BlendStateDesc& Desc, BlendState** ppBlendState) { return false; }
		virtual bool CreateDepthStencilState(const DepthStencilStateDesc& Desc, DepthStencilState** ppDepthStencilState) { return false; }
		virtual bool CreateRasterizerState(const RasterizerStateDesc& Desc, RasterizerState** ppRasterizerState) { return false; }

		bool CreateBuffer(const BufferDesc& Desc, SubresourceData* pInitialData, Buffer** ppBuffer) override;
		virtual void BindBufferBase(Buffer* pBuffer, uint32 Index) {}
		virtual void BindBufferRange(Buffer* pBuffer, uint32 Index, uint32 Offset, uint32 Size) {}
		void MapBuffer(Buffer* pBuffer, BufferMapAccess MapAccess, void*& MappedData) override;
		void UnmapBuffer(Buffer* pBuffer) override;

		bool CreateGraphicsPipeline(const GraphicsPipelineDesc& Desc, GraphicsPipeline** ppPipeline) override;
		virtual bool CreateComputePipelineState(const ComputePipelineDesc& Desc, ComputePipeline** ppComputePipelineState) { return false; }

		void Dispatch(uint32 X, uint32 Y, uint32 Z) override;
		void Draw(uint32 VertexCount, uint32 StartVertexLocation) override;
		void DrawIndexed(uint32 IndexCount, uint32 StartIndexLocation, int BaseVertexLocation) override;

		void BeginMarker(const char* Str) override;
		void EndMarker() override;

		virtual ShaderProgram* CreateShaderProgram() const { return nullptr; }
		virtual Texture* CreateTexture() const { return nullptr; }
		virtual Mesh* CreateMesh() const { return nullptr; }
		virtual Framebuffer* CreateFramebuffer() const { return nullptr; }

	};

	class DeviceDX12
	{
	private:
	public:
		ID3D12Device* _device;
		ID3D12CommandQueue* _queue;
		ID3D12GraphicsCommandList* _cmdList;

		void BeginFrame();
		void EndFrame();

		bool CreateComputePipeline(const ComputePipelineDesc& Desc, ComputePipeline** ppPipeline);
	};

}
