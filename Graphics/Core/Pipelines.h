#pragma once

#include <Graphics/Core/Types.h>
#include <Graphics/Core/InputLayout.h>
#include <ShaderBytecode/ShaderBytecode.h>
#include <Core/Core.h>

namespace Columbus
{

	struct ComputePipelineDesc
	{
		std::string Name; // Debug name
		CompiledShaderData Bytecode;
	};

	struct GraphicsPipelineDesc
	{
		InputLayout layout;
		DepthStencilStateDesc depthStencilState;
		RasterizerStateDesc rasterizerState;
		BlendStateDesc blendState;
		PrimitiveTopology topology = PrimitiveTopology::TriangleList;

		std::string Name;
		CompiledShaderData Bytecode;
	};

	struct RayTracingPipelineDesc
	{
		// TODO: Shader Binding Table description

		uint32_t MaxRecursionDepth;

		std::string Name;
		CompiledShaderData Bytecode;
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
