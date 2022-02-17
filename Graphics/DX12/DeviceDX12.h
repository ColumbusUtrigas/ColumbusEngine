#pragma once

#include <Graphics/Device.h>
#include <Graphics/GraphicsPipeline.h>
#include <Graphics/ComputePipeline.h>
#include <Graphics/Buffer.h>
#include <Graphics/Types.h>
#include <d3d12.h>

namespace Columbus::Graphics::DX12
{

	class DeviceDX12 : public Device
	{
	private:
		ID3D12Device* _device;
		ID3D12CommandQueue* _queue;
		ID3D12GraphicsCommandList* _cmdList;
		ID3D12DescriptorHeap* _srvHeap;
	public:
		DeviceDX12(ID3D12Device* device, ID3D12CommandQueue* queue, ID3D12GraphicsCommandList* cmdList, ID3D12DescriptorHeap* srvHeap) :
			_device(device), _queue(queue), _cmdList(cmdList), _srvHeap(srvHeap)
		{
			_currentApi = GraphicsAPI::DX12;
		}

		// LEGACY
		virtual void IASetInputLayout(InputLayout* Layout) {}
		virtual void OMSetBlendState(BlendState* pBlendState, const float BlendFactor[4], uint32 SampleMask) {}
		virtual void OMSetDepthStencilState(DepthStencilState* pDepthStencilState, uint32 StencilRef) {}
		virtual void RSSetState(RasterizerState* pRasterizerState) {}
		virtual void SetShader(ShaderProgram* Prog) {}
		virtual void SetComputePipelineState(ComputePipeline* State) {}
		virtual bool CreateBlendState(const BlendStateDesc& Desc, BlendState** ppBlendState) { return false; }
		virtual bool CreateDepthStencilState(const DepthStencilStateDesc& Desc, DepthStencilState** ppDepthStencilState) { return false; }
		virtual bool CreateRasterizerState(const RasterizerStateDesc& Desc, RasterizerState** ppRasterizerState) { return false; }
		virtual void BindBufferBase(Buffer* pBuffer, uint32 Index) {}
		virtual void BindBufferRange(Buffer* pBuffer, uint32 Index, uint32 Offset, uint32 Size) {}
		virtual bool CreateComputePipelineState(const ComputePipelineDesc& Desc, ComputePipeline** ppComputePipelineState) { return false; }
		virtual ShaderProgram* CreateShaderProgram() const { return nullptr; }
		virtual Texture* CreateTexture() const { return nullptr; }
		virtual Mesh* CreateMesh() const { return nullptr; }
		virtual Framebuffer* CreateFramebuffer() const { return nullptr; }
		/////////////////////////////////////

		ID3D12Resource* CreateTextureDx();

		void IASetVertexBuffers(uint32 StartSlot, uint32 NumBuffers, Buffer** ppBuffers) override;
		void IASetIndexBuffer(Buffer* pIndexBuffer, IndexFormat Format, uint32 Offset) override;

		void RSSetViewports(uint32 Num, Viewport* pViewports) override;
		void RSSetScissorRects(uint32 Num, ScissorRect* pScissors) override;

		void SetGraphicsPipeline(GraphicsPipeline* pPipeline) override;
		void SetGraphicsCBV(uint32 slot, Buffer* pBuf) override;

		void SetComputePipeline(ComputePipeline* pPipeline) override;

		bool CreateBuffer(const BufferDesc& Desc, SubresourceData* pInitialData, Buffer** ppBuffer) override;
		void MapBuffer(Buffer* pBuffer, BufferMapAccess MapAccess, void*& MappedData) override;
		void UnmapBuffer(Buffer* pBuffer) override;

		bool CreateGraphicsPipeline(const GraphicsPipelineDesc& Desc, GraphicsPipeline** ppPipeline) override;
		bool CreateComputePipeline(const ComputePipelineDesc& desc, ComputePipeline** ppPipeline) override;

		void Dispatch(uint32 X, uint32 Y, uint32 Z) override;
		void Draw(uint32 VertexCount, uint32 StartVertexLocation) override;
		void DrawIndexed(uint32 IndexCount, uint32 StartIndexLocation, int BaseVertexLocation) override;

		void BeginMarker(const char* Str) override;
		void EndMarker() override;
	};

}
