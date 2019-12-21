#pragma once

#include <string>

namespace Columbus
{

	struct ComputePipelineStateDesc
	{
		std::string CS; // source code!
	};

	class ComputePipelineState
	{
	public:
		ComputePipelineStateDesc Desc;
	public:
		const ComputePipelineStateDesc& GetDesc() const { return Desc; }
	};

}
