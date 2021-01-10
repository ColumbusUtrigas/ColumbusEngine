#pragma once

#include <Graphics/GraphicsPipeline.h>
#include <gl/glew.h>

namespace Columbus::Graphics::GL
{

	class GraphicsPipelineGL : public GraphicsPipeline
	{
	private:
		friend class DeviceOpenGL;

		GLuint _vs;
		GLuint _ps;
		GLuint _prog;
	public:
		GraphicsPipelineGL(const GraphicsPipelineDesc& Desc) :
			GraphicsPipeline(Desc) {}
	};

}
