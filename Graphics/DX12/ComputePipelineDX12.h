#pragma once

#include <Graphics/ComputePipeline.h>
#include <d3d12.h>

namespace Columbus::Graphics::DX12
{

	class ComputePipelineDX12 : public ComputePipeline
	{
	public:
		ID3D12PipelineState* _pso = nullptr;
		ID3D12RootSignature* _rso = nullptr;
	public:
		ComputePipelineDX12(const ComputePipelineDesc& desc) :
			ComputePipeline(desc) {}
	};

}
