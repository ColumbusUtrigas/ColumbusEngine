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

	void DeviceDX12::IASetVertexBuffers(uint32 StartSlot, uint32 NumBuffers, Buffer** ppBuffers)
	{
		static D3D12_VERTEX_BUFFER_VIEW views[16];
		for (size_t i = 0; i < NumBuffers; i++)
		{
			views[i].BufferLocation = static_cast<BufferDX12*>(ppBuffers[i])->_resource->GetGPUVirtualAddress();
			views[i].SizeInBytes = ppBuffers[i]->GetDesc().Size;
			views[i].StrideInBytes = ppBuffers[i]->GetDesc().StructureByteStride;;
		}

		_cmdList->IASetVertexBuffers(StartSlot, NumBuffers, views);
	}

	void DeviceDX12::IASetIndexBuffer(Buffer* pIndexBuffer, IndexFormat Format, uint32 Offset)
	{
		D3D12_INDEX_BUFFER_VIEW view;
		view.BufferLocation = static_cast<BufferDX12*>(pIndexBuffer)->_resource->GetGPUVirtualAddress();
		view.SizeInBytes = pIndexBuffer->GetDesc().Size;
		view.Format = IndexFormatToDX12(Format);

		_cmdList->IASetIndexBuffer(&view);
	}

	void DeviceDX12::RSSetViewports(uint32 Num, Viewport* pViewports)
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

	void DeviceDX12::RSSetScissorRects(uint32 Num, ScissorRect* pScissors)
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

	void DeviceDX12::SetGraphicsPipeline(GraphicsPipeline* pPipeline)
	{
		auto pipe = static_cast<GraphicsPipelineDX12*>(pPipeline);
		_cmdList->SetGraphicsRootSignature(pipe->_rso);
		_cmdList->SetPipelineState(pipe->_pso);
		_cmdList->IASetPrimitiveTopology(PrimitiveTopologyToDX12(pipe->GetDesc().topology));
		_cmdList->SetGraphicsRootDescriptorTable(1, _srvHeap->GetGPUDescriptorHandleForHeapStart());
	}

	void DeviceDX12::SetGraphicsCBV(uint32 slot, Buffer* pBuf)
	{
		_cmdList->SetGraphicsRootConstantBufferView(slot, static_cast<BufferDX12*>(pBuf)->_resource->GetGPUVirtualAddress());
	}

	void DeviceDX12::SetComputePipeline(ComputePipeline* pPipeline)
	{
		auto pipe = static_cast<ComputePipelineDX12*>(pPipeline);
		_cmdList->SetComputeRootSignature(pipe->_rso);
		_cmdList->SetPipelineState(pipe->_pso);
	}

	bool DeviceDX12::CreateBuffer(const BufferDesc& Desc, SubresourceData* pInitialData, Buffer** ppBuffer)
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

	ID3D12Resource* DeviceDX12::CreateTextureDx()
	{
		auto img = Image("Data/Textures/lantern_Albedo.png");
		auto width = 2;
		auto height = 1;

		width = img.GetWidth();
		height = img.GetHeight();

		D3D12_HEAP_PROPERTIES props;
		memset(&props, 0, sizeof(D3D12_HEAP_PROPERTIES));
		props.Type = D3D12_HEAP_TYPE_DEFAULT;
		props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		D3D12_RESOURCE_DESC desc;
		memset(&desc, 0, sizeof(D3D12_RESOURCE_DESC));
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Width = width;
		desc.Height = height;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;

		ID3D12Resource* handle;
		auto hr = _device->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COPY_DEST, NULL, IID_PPV_ARGS(&handle));

		/*UINT uploadPitch = (width * 4 + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u) & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u);
		UINT uploadSize = height * uploadPitch;
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Alignment = 0;
		desc.Width = uploadSize;
		desc.Height = 1;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;

		props.Type = D3D12_HEAP_TYPE_UPLOAD;
		props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		ID3D12Resource* uploadBuffer;
		hr = _device->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, IID_PPV_ARGS(&uploadBuffer));

		char tex[] = {
			66, 135, 245, 255,
			167, 66, 245, 256
		};

		void* mapped = NULL;
		D3D12_RANGE range = { 0, uploadSize };
		hr = uploadBuffer->Map(0, &range, &mapped);
		memcpy(mapped, tex, width * 4);
		uploadBuffer->Unmap(0, &range);*/

		UINT uploadPitch = (img.GetWidth() * 4 + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u) & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u);
		UINT uploadSize = img.GetHeight() * uploadPitch;
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Alignment = 0;
		desc.Width = uploadSize;
		desc.Height = 1;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;

		props.Type = D3D12_HEAP_TYPE_UPLOAD;
		props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		ID3D12Resource* uploadBuffer;
		hr = _device->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, IID_PPV_ARGS(&uploadBuffer));

		void* mapped = NULL;
		D3D12_RANGE range = { 0, uploadSize };
		hr = uploadBuffer->Map(0, &range, &mapped);
		for (int y = 0; y < img.GetHeight(); y++)
			memcpy((void*)((uintptr_t)mapped + y * uploadPitch), img.GetData() + y * img.GetWidth() * 4, img.GetWidth() * 4);
		uploadBuffer->Unmap(0, &range);

		D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
		srcLocation.pResource = uploadBuffer;
		srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		srcLocation.PlacedFootprint.Footprint.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srcLocation.PlacedFootprint.Footprint.Width = width;
		srcLocation.PlacedFootprint.Footprint.Height = height;
		srcLocation.PlacedFootprint.Footprint.Depth = 1;
		srcLocation.PlacedFootprint.Footprint.RowPitch = uploadPitch;

		D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
		dstLocation.pResource = handle;
		dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dstLocation.SubresourceIndex = 0;

		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = handle;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

		ID3D12Fence* fence = NULL;
		hr = _device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
		HANDLE event = CreateEvent(0, 0, 0, 0);

		_cmdList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, NULL);
		_cmdList->ResourceBarrier(1, &barrier);

		_queue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&_cmdList);
		hr = _queue->Signal(fence, 1);
		//hr = _cmdList->Close();

		fence->SetEventOnCompletion(1, event);
		WaitForSingleObject(event, INFINITE);

		/*cmdList->Release();
		cmdAlloc->Release();
		cmdQueue->Release();
		CloseHandle(event);
		fence->Release();
		uploadBuffer->Release();*/

		// Create texture view
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(srvDesc));
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = desc.MipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		_device->CreateShaderResourceView(handle, &srvDesc, _srvHeap->GetCPUDescriptorHandleForHeapStart());

		return handle;
	}

	void DeviceDX12::MapBuffer(Buffer* pBuffer, BufferMapAccess MapAccess, void*& MappedData)
	{
		static_cast<ID3D12Resource*>(pBuffer->GetHandle())->Map(0, nullptr, &MappedData);
	}

	void DeviceDX12::UnmapBuffer(Buffer* pBuffer)
	{
		static_cast<ID3D12Resource*>(pBuffer->GetHandle())->Unmap(0, nullptr);
	}

	bool DeviceDX12::CreateGraphicsPipeline(const GraphicsPipelineDesc& Desc, GraphicsPipeline** ppPipeline)
	{
		GraphicsPipelineDX12* pipeline = new GraphicsPipelineDX12(Desc);
		*ppPipeline = pipeline;

		// root signature
		{
			fixed_vector<D3D12_ROOT_PARAMETER, 64> params;

			for (auto& cb : Desc.cbs)
			{
				D3D12_ROOT_PARAMETER param{};
				param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
				param.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

				params.push_back(param);
			}

			D3D12_DESCRIPTOR_RANGE ranges[1]{};
			ranges[0].BaseShaderRegister = 0;
			ranges[0].NumDescriptors = 1;
			ranges[0].RegisterSpace = 0;
			ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			ranges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			D3D12_ROOT_PARAMETER param{};
			//param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
			//param.Descriptor.ShaderRegister = 0;
			//param.Descriptor.RegisterSpace = 0;
			param.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
			param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			param.DescriptorTable.NumDescriptorRanges = 1;
			param.DescriptorTable.pDescriptorRanges = ranges;

			params.push_back(param);

			D3D12_STATIC_SAMPLER_DESC samplers[1]{};
			samplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
			samplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
			samplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
			samplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
			samplers[0].MipLODBias = 0;
			samplers[0].MaxAnisotropy = 0;
			samplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
			samplers[0].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
			samplers[0].MinLOD = 0.0f;
			samplers[0].MaxLOD = D3D12_FLOAT32_MAX;
			samplers[0].ShaderRegister = 0;
			samplers[0].RegisterSpace = 0;
			samplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

			D3D12_ROOT_SIGNATURE_DESC desc;
			desc.NumParameters = params.size();
			desc.pParameters = params.data();
			desc.NumStaticSamplers = 1;
			desc.pStaticSamplers = samplers;
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

	bool DeviceDX12::CreateComputePipeline(const ComputePipelineDesc& desc, ComputePipeline** ppPipeline)
	{
		ComputePipelineDX12* pipeline = new ComputePipelineDX12(desc);
		*ppPipeline = pipeline;

		// root signature
		{

		}

		auto cs = CompileShaderStage_DX12(desc.CS, "cs_5_0");

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

	void DeviceDX12::Dispatch(uint32 X, uint32 Y, uint32 Z)
	{
		_cmdList->Dispatch(X, Y, Z);
	}

	void DeviceDX12::Draw(uint32 VertexCount, uint32 StartVertexLocation)
	{
		_cmdList->DrawInstanced(VertexCount, 1, StartVertexLocation, 0);
	}

	void DeviceDX12::DrawIndexed(uint32 IndexCount, uint32 StartIndexLocation, int BaseVertexLocation)
	{
		_cmdList->DrawIndexedInstanced(IndexCount, 1, StartIndexLocation, BaseVertexLocation, 0);
	}

	void DeviceDX12::BeginMarker(const char* Str)
	{
		PIXBeginEvent(_cmdList, PIX_COLOR_DEFAULT, Str);
	}

	void DeviceDX12::EndMarker()
	{
		PIXEndEvent(_cmdList);
	}

}
