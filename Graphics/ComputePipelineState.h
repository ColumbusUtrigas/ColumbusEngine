#pragma once

#include <string>

#include <GL/glew.h>

namespace Columbus
{

	struct ComputePipelineStateDesc
	{
		std::string CS; // source code!
	};

	class ComputePipelineState
	{
	private:
		ComputePipelineStateDesc Desc;
	public:
		GLuint progid;
		GLuint shadid;
	public:
		ComputePipelineState(const ComputePipelineStateDesc& Desc) : Desc(Desc) {}
		const ComputePipelineStateDesc& GetDesc() const { return Desc; }
	};

}
