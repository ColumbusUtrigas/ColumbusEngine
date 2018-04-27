#include <RenderAPIOpenGL/OpenGL.h>

namespace Columbus
{

	int32 OpenGL::MaxCombinedTextureImageUnits = -1;
	int32 OpenGL::MaxCubemapTextureSize = -1;
	int32 OpenGL::MaxFragmentUniformVectors = -1;
	int32 OpenGL::MaxRenderbufferSize = -1;
	int32 OpenGL::MaxTextureImageUnits = -1;
	int32 OpenGL::MaxTextureSize = -1;
	int32 OpenGL::MaxVaryingVectors = -1;
	int32 OpenGL::MaxVertexAttribs = -1;
	int32 OpenGL::MaxVertexTextureImageUnits = -1;
	int32 OpenGL::MaxVertexUniformVectors = -1;
	int32 OpenGL::MaxViewportDims = -1;

	bool OpenGL::bSupportsShader = false;
	bool OpenGL::bSupportsGeometryShader = false;
	bool OpenGL::bSupportsTesselation = false;
	bool OpenGL::bSupportsComputeShader = false;

	void OpenGL::Init()
	{
		glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &MaxCombinedTextureImageUnits);
		glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &MaxCubemapTextureSize);
		glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_VECTORS, &MaxFragmentUniformVectors);
		glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &MaxRenderbufferSize);
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &MaxTextureImageUnits);
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &MaxTextureSize);
		glGetIntegerv(GL_MAX_VARYING_VECTORS, &MaxVaryingVectors);
		glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &MaxVertexAttribs);
		glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &MaxVertexTextureImageUnits);
		glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, &MaxVertexUniformVectors);
		glGetIntegerv(GL_MAX_VIEWPORT_DIMS, &MaxViewportDims);

		if (glewIsSupported("GL_ARB_vertex_program GL_ARB_fragment_program") == GL_TRUE)
		{
			bSupportsShader = true;
		}

		if (glewIsSupported("GL_ARB_geometry_shader4") == GL_TRUE)
		{
			bSupportsGeometryShader = true;
		}

		if (glewIsSupported("GL_ARB_tessellation_shader") == GL_TRUE)
		{
			bSupportsTesselation = true;
		}

		if (glewIsSupported("GL_ARB_compute_shader") == GL_TRUE)
		{
			bSupportsComputeShader = true;
		}
	}

}





