#pragma once

#include <Graphics/ComputePipeline.h>
#include <GL/glew.h>

namespace Columbus::Graphics::GL
{

	class ComputePipelineGL : public ComputePipeline
	{
	public:
		GLuint progid;
		GLuint shadid;

		ComputePipelineGL(const ComputePipelineDesc& desc) :
			ComputePipeline(desc) {}
	};

}
