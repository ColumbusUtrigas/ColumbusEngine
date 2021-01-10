#pragma once

#include <Graphics/ComputePipeline.h>
#include <GL/glew.h>

namespace Columbus::Graphics::GL
{

	class ComputePipeline : public Columbus::ComputePipeline
	{
	public:
		GLuint progid;
		GLuint shadid;

		ComputePipeline(const ComputePipelineDesc& Desc) :
			Columbus::ComputePipeline(Desc) {}
	};

}
