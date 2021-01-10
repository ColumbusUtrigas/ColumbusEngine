#pragma once

#include <Graphics/Types.h>
#include <Core/SmartPointer.h>

namespace Columbus
{

	struct ComputePipelineDesc
	{
		SPtr<ShaderStage> CS;
	};

	class ComputePipeline
	{
	private:
		ComputePipelineDesc Desc;
	public:
		ComputePipeline(const ComputePipelineDesc& Desc) : Desc(Desc) {}
		const ComputePipelineDesc& GetDesc() const { return Desc; }
	};

}
