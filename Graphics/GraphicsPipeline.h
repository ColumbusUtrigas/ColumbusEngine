#pragma once

#include <Graphics/InputLayout.h>
#include <Graphics/DepthStencilState.h>
#include <Graphics/RasterizerState.h>
#include <Graphics/BlendState.h>
#include <Core/SmartPointer.h>

namespace Columbus::Graphics
{

	struct ShaderResourceTex2D
	{
		std::string name;
		int slot;
	};

	struct ShaderResourceDesc
	{
		const char* name;
		int slot;
	};

	struct GraphicsPipelineDesc
	{
		InputLayout layout;
		DepthStencilStateDesc depthStencilState;
		RasterizerStateDesc rasterizerState;
		BlendStateDesc blendState;
		PrimitiveTopology topology = PrimitiveTopology::Undefined;

		fixed_vector<ShaderResourceTex2D, 16> textures;
		fixed_vector<ShaderResourceDesc, 16> cbs;

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
