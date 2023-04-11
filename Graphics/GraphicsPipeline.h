#pragma once

#include <Graphics/InputLayout.h>
#include <Graphics/DepthStencilState.h>
#include <Graphics/RasterizerState.h>
#include <Graphics/BlendState.h>
#include <Core/SmartPointer.h>

namespace Columbus
{

	struct GraphicsPipelineDesc
	{
		InputLayout layout;
		DepthStencilStateDesc depthStencilState;
		RasterizerStateDesc rasterizerState;
		BlendStateDesc blendState;
		PrimitiveTopology topology = PrimitiveTopology::TriangleList;

		std::string Name;

		SPtr<ShaderStage> VS;
		SPtr<ShaderStage> HS;
		SPtr<ShaderStage> DS;
		SPtr<ShaderStage> GS;
		SPtr<ShaderStage> PS;
	};

	class GraphicsPipeline
	{
	protected:
		GraphicsPipelineDesc _desc;
	public:
		GraphicsPipeline(const GraphicsPipelineDesc& desc) : _desc(desc) {}
		const GraphicsPipelineDesc& GetDesc() const { return _desc; }
	};

}
