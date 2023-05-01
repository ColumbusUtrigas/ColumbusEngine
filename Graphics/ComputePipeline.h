#pragma once

#include <Graphics/Types.h>
#include <Core/SmartPointer.h>
#include <string>

namespace Columbus
{

	struct ComputePipelineDesc
	{
		SPtr<ShaderStage> CS;
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

}
