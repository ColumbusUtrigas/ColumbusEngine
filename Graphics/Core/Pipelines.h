#pragma once

#include <Graphics/Core/Types.h>
#include <Graphics/Core/InputLayout.h>
#include <Core/Core.h>

namespace Columbus
{

	struct ComputePipelineDesc
	{
		SPtr<ShaderStage> CS;
		std::string Name;
	};

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

	struct RayTracingPipelineDesc
	{
		SPtr<ShaderStage> RayGen;
		SPtr<ShaderStage> Miss;
		SPtr<ShaderStage> ClosestHit;

		uint32_t MaxRecursionDepth;

		std::string Name;
	};

	class ComputePipeline
	{
	private:
		ComputePipelineDesc _Desc;
	public:
		ComputePipeline(const ComputePipelineDesc& Desc) : _Desc(Desc) {}
		const ComputePipelineDesc& GetDesc() const { return _Desc; }
	};

	class GraphicsPipeline
	{
	protected:
		GraphicsPipelineDesc _desc;
	public:
		GraphicsPipeline(const GraphicsPipelineDesc& desc) : _desc(desc) {}
		const GraphicsPipelineDesc& GetDesc() const { return _desc; }
	};

	class RayTracingPipeline
	{
	private:
		RayTracingPipelineDesc _Desc;
	public:
		RayTracingPipeline(const RayTracingPipelineDesc& Desc) : _Desc(Desc) {}
		const RayTracingPipelineDesc& GetDesc() const { return _Desc; }
	};

}
