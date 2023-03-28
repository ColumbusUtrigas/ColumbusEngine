#pragma once

#include <Graphics/GraphicsPipeline.h>
#include <GL/glew.h>

namespace Columbus::Graphics::GL
{

	class GraphicsPipelineGL : public GraphicsPipeline
	{
	private:
		friend class DeviceOpenGL;

	public:
		GLuint _vs;
		GLuint _ps;
		GLuint _prog;
	public:
		GraphicsPipelineGL(const GraphicsPipelineDesc& Desc) :
			GraphicsPipeline(Desc) {}
	};

}
