#include <Graphics/GraphicsPipeline.h>
#include <d3d12.h>

namespace Columbus::Graphics::DX12
{

	class GraphicsPipelineDX12 : public GraphicsPipeline
	{
	public:
		ID3D12PipelineState* _pso = nullptr;
		ID3D12RootSignature* _rso = nullptr;
	public:
		GraphicsPipelineDX12(const GraphicsPipelineDesc& Desc) :
			GraphicsPipeline(Desc) {}
	};

}
