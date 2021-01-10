#include <Graphics/DX12/DeviceDX12.h>
#include <Graphics/DX12/BufferDX12.h>
#include <Graphics/DX12/GraphicsPipelineDX12.h>
#include <Graphics/DX12/ComputePipelineDX12.h>
#include <Graphics/DX12/TypeConversions.h>
#include <Graphics/DX12/ShaderCompile.h>
#include <Core/Assert.h>
#include <vector>

#include <pix3.h>

namespace Columbus::Graphics::DX12
{

	void DeviceDX12::BeginFrame()
	{

	}

	void DeviceDX12::EndFrame()
	{
		_cmdList->Close();
		_queue->ExecuteCommandLists(1, (ID3D12CommandList**)&_cmdList);
	}

	bool DeviceDX12::CreateComputePipeline(const ComputePipelineDesc& Desc, ComputePipeline** ppPipeline)
	{
		ComputePipelineDX12* pipeline = new ComputePipelineDX12(Desc);
		*ppPipeline = pipeline;

		// root signature
		{

		}

		auto cs = CompileShaderStage_DX12(Desc.CS, "cs_5_0");

		// pso
		{
			D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
			desc.pRootSignature = pipeline->_rso;
			desc.CS = *cs;
			desc.NodeMask = 0;
			desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

			return _device->CreateComputePipelineState(&desc, IID_PPV_ARGS(&pipeline->_pso)) == S_OK;
		}
	}

	void tmpDeviceDX12::IASetVertexBuffers(uint32 StartSlot, uint32 NumBuffers, Buffer** ppBuffers)
	{
		uint32 stides[] = { 2 * sizeof(float), 4 * sizeof(float) };

		static D3D12_VERTEX_BUFFER_VIEW views[16];
		for (size_t i = 0; i < NumBuffers; i++)
		{
			views[i].BufferLocation = static_cast<BufferDX12*>(ppBuffers[i])->_resource->GetGPUVirtualAddress();
			views[i].SizeInBytes = ppBuffers[i]->GetDesc().Size;
			views[i].StrideInBytes = stides[i];
		}

		_cmdList->IASetVertexBuffers(StartSlot, NumBuffers, views);
	}

	void tmpDeviceDX12::IASetIndexBuffer(Buffer* pIndexBuffer, IndexFormat Format, uint32 Offset)
	{
		D3D12_INDEX_BUFFER_VIEW view;
		view.BufferLocation = static_cast<BufferDX12*>(pIndexBuffer)->_resource->GetGPUVirtualAddress();
		view.SizeInBytes = pIndexBuffer->GetDesc().Size;
		view.Format = IndexFormatToDX12(Format);

		_cmdList->IASetIndexBuffer(&view);
	}

	void tmpDeviceDX12::RSSetViewports(uint32 Num, Viewport* pViewports)
	{
		static std::vector<D3D12_VIEWPORT> pool(16);
		pool.clear();

		for (uint32 i = 0; i < Num; i++)
		{
			pool.push_back(D3D12_VIEWPORT{});
			auto& v = pool.back();

			v.TopLeftX = pViewports[i].TopLeftX;
			v.TopLeftY = pViewports[i].TopLeftY;
			v.Width = pViewports[i].Width;
			v.Height = pViewports[i].Height;
			v.MinDepth = pViewports[i].MinDepth;
			v.MaxDepth = pViewports[i].MaxDepth;
		}

		_cmdList->RSSetViewports(Num, pool.data());
	}

	void tmpDeviceDX12::RSSetScissorRects(uint32 Num, ScissorRect* pScissors)
	{
		static std::vector<D3D12_RECT> pool(16);
		pool.clear();

		for (uint32 i = 0; i < Num; i++)
		{
			pool.push_back(D3D12_RECT{});
			auto& r = pool.back();

			r.left = pScissors[i].Left;
			r.top = pScissors[i].Top;
			r.right = pScissors[i].Right;
			r.bottom = pScissors[i].Bottom;
		}

		_cmdList->RSSetScissorRects(Num, pool.data());
	}

	void tmpDeviceDX12::SetGraphicsPipeline(GraphicsPipeline* pPipeline)
	{
		auto pipe = static_cast<GraphicsPipelineDX12*>(pPipeline);
		_cmdList->SetGraphicsRootSignature(pipe->_rso);
		_cmdList->SetPipelineState(pipe->_pso);
		_cmdList->IASetPrimitiveTopology(PrimitiveTopologyToDX12(pipe->GetDesc().topology));
	}

	bool tmpDeviceDX12::CreateBuffer(const BufferDesc& Desc, SubresourceData* pInitialData, Buffer** ppBuffer)
	{
		*ppBuffer = new BufferDX12(Desc);

		D3D12_HEAP_PROPERTIES props;
		memset(&props, 0, sizeof(D3D12_HEAP_PROPERTIES));
		props.Type = D3D12_HEAP_TYPE_UPLOAD;
		props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		D3D12_RESOURCE_DESC desc;
		memset(&desc, 0, sizeof(D3D12_RESOURCE_DESC));
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Width = Desc.Size;
		desc.Height = 1;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.SampleDesc.Count = 1;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;

		auto& handle = static_cast<BufferDX12*>(*ppBuffer)->_resource;
		auto hr = _device->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, IID_PPV_ARGS(&handle));

		if (FAILED(hr))
		{
			delete (*ppBuffer);
			return false;
		}

		if (pInitialData != nullptr)
		{
			void* data;
			MapBuffer(*ppBuffer, BufferMapAccess::Write, data);
			memcpy(data, pInitialData->pSysMem, pInitialData->SysMemPitch);
			UnmapBuffer(*ppBuffer);
		}

		return true;
	}

	void tmpDeviceDX12::MapBuffer(Buffer* pBuffer, BufferMapAccess MapAccess, void*& MappedData)
	{
		static_cast<ID3D12Resource*>(pBuffer->GetHandle())->Map(0, nullptr, &MappedData);
	}

	void tmpDeviceDX12::UnmapBuffer(Buffer* pBuffer)
	{
		static_cast<ID3D12Resource*>(pBuffer->GetHandle())->Unmap(0, nullptr);
	}

	bool tmpDeviceDX12::CreateGraphicsPipeline(const GraphicsPipelineDesc& Desc, GraphicsPipeline** ppPipeline)
	{
		GraphicsPipelineDX12* pipeline = new GraphicsPipelineDX12(Desc);
		*ppPipeline = pipeline;

		// root signature
		{
			D3D12_ROOT_PARAMETER param[1] = {};

			param[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
			param[0].Constants.ShaderRegister = 0;
			param[0].Constants.RegisterSpace = 0;
			param[0].Constants.Num32BitValues = 4;
			param[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

			D3D12_ROOT_SIGNATURE_DESC desc;
			desc.NumParameters = 0;
			desc.pParameters = param;
			desc.NumStaticSamplers = 0;
			desc.pStaticSamplers = nullptr;
			desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

			ID3DBlob* signature;
			auto hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, nullptr);
			if (FAILED(hr))
			{
				return false;
			}

			hr = _device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&pipeline->_rso));
			if (FAILED(hr))
			{
				return false;
			}
		}

		auto empty = D3D12_SHADER_BYTECODE{};
		auto vs = CompileShaderStage_DX12(Desc.VS, "vs_5_0");
		auto hs = CompileShaderStage_DX12(Desc.HS, "hs_5_0");
		auto ds = CompileShaderStage_DX12(Desc.DS, "ds_5_0");
		auto gs = CompileShaderStage_DX12(Desc.GS, "gs_5_0");
		auto ps = CompileShaderStage_DX12(Desc.PS, "ps_5_0");


		// pso
		{
			D3D12_INPUT_ELEMENT_DESC elements[16];

			D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
			desc.InputLayout = InputLayoutToDX12(Desc.layout, elements);
			desc.pRootSignature = pipeline->_rso;
			desc.VS = *vs;
			desc.HS = hs ? *hs : empty;
			desc.DS = ds ? *ds : empty;
			desc.GS = gs ? *gs : empty;
			desc.PS = ps ? *ps : empty;
			desc.PrimitiveTopologyType = PrimitiveTopologyTypeToDX12(Desc.topology);
			desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.SampleMask = 0xffffffff;
			desc.RasterizerState = RasterizerStateToDX12(Desc.rasterizerState);
			desc.BlendState = BlendStateToDX12(Desc.blendState);
			desc.DepthStencilState = DepthStencilStateToDX12(Desc.depthStencilState);
			desc.NumRenderTargets = 1;

			auto hr = _device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pipeline->_pso));
			return hr == S_OK;
		}
	}

	void tmpDeviceDX12::Dispatch(uint32 X, uint32 Y, uint32 Z)
	{
		_cmdList->Dispatch(X, Y, Z);
	}

	void tmpDeviceDX12::Draw(uint32 VertexCount, uint32 StartVertexLocation)
	{
		_cmdList->DrawInstanced(VertexCount, 1, StartVertexLocation, 0);
	}

	void tmpDeviceDX12::DrawIndexed(uint32 IndexCount, uint32 StartIndexLocation, int BaseVertexLocation)
	{
		_cmdList->DrawIndexedInstanced(IndexCount, 1, StartIndexLocation, BaseVertexLocation, 0);
	}

	void tmpDeviceDX12::BeginMarker(const char* Str)
	{
		PIXBeginEvent(_cmdList, PIX_COLOR_DEFAULT, Str);
	}

	void tmpDeviceDX12::EndMarker()
	{
		PIXEndEvent(_cmdList);
	}

}
