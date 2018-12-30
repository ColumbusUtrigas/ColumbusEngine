#include <RenderAPIOpenGL/OpenGL.h>
#include <GL/glew.h>

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
	int32 OpenGL::MaxComputeWorkGroupInvocations = -1;
	int32 OpenGL::MaxComputeSharedMemorySize = -1;
	iVector3 OpenGL::MaxComputeWorkGroupSize = iVector3(-1);
	iVector3 OpenGL::MaxComputeWorkGroupCount = iVector3(-1);

	bool OpenGL::bSupportsUniformBuffer = false;
	bool OpenGL::bSupportsInstancing = false;
	bool OpenGL::bSupportsTransformFeedback = false;
	bool OpenGL::bSupportsShader = false;
	bool OpenGL::bSupportsGeometryShader = false;
	bool OpenGL::bSupportsTesselation = false;
	bool OpenGL::bSupportsComputeShader = false;
	bool OpenGL::bSupportsDirectStateAccess = false;
	bool OpenGL::bSupportsProgramBinary = false;
	bool OpenGL::bSupportsShaderStorageBuffer = false;
	bool OpenGL::bSupportsShaderImageLoadStore = false;
	bool OpenGL::bSupportsMultiBind = false;
	bool OpenGL::bSupportsBaseInstance = false;
	bool OpenGL::bSupportsDrawIndirect = false;
	bool OpenGL::bSupportsMultiDrawIndirect = false;

	void OpenGL::Init()
	{
		bSupportsUniformBuffer        = glewGetExtension("GL_ARB_uniform_buffer_object");
		bSupportsInstancing           = glewGetExtension("GL_ARB_instanced_arrays") && (glewGetExtension("GL_ARB_draw_instanced") || glewGetExtension("GL_EXT_draw_instanced"));
		bSupportsTransformFeedback    = glewGetExtension("GL_ARB_transform_feedback2");
		bSupportsShader               = glewGetExtension("GL_ARB_vertex_program") && glewGetExtension("GL_ARB_fragment_program");
		bSupportsGeometryShader       = glewGetExtension("GL_ARB_geometry_shader4");
		bSupportsTesselation          = glewGetExtension("GL_ARB_tessellation_shader");
		bSupportsComputeShader        = glewGetExtension("GL_ARB_compute_shader");
		bSupportsDirectStateAccess    = glewGetExtension("GL_ARB_direct_state_access") || glewGetExtension("GL_EXT_direct_state_access");
		bSupportsProgramBinary        = glewGetExtension("GL_ARB_get_program_binary");
		bSupportsShaderStorageBuffer  = glewGetExtension("GL_ARB_shader_storage_buffer_object");
		bSupportsShaderImageLoadStore = glewGetExtension("GL_ARB_shader_image_load_store") || glewGetExtension("GL_EXT_shader_image_load_store");
		bSupportsMultiBind            = glewGetExtension("GL_ARB_multi_bind");
		bSupportsBaseInstance         = glewGetExtension("GL_ARB_base_instance");
		bSupportsDrawIndirect         = glewGetExtension("GL_ARB_draw_indirect");
		bSupportsMultiDrawIndirect    = glewGetExtension("GL_ARB_multi_draw_indirect");

		glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &MaxCombinedTextureImageUnits);
		glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE,        &MaxCubemapTextureSize);
		glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_VECTORS,     &MaxFragmentUniformVectors);
		glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE,            &MaxRenderbufferSize);
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS,          &MaxTextureImageUnits);
		glGetIntegerv(GL_MAX_TEXTURE_SIZE,                 &MaxTextureSize);
		glGetIntegerv(GL_MAX_VARYING_VECTORS,              &MaxVaryingVectors);
		glGetIntegerv(GL_MAX_VERTEX_ATTRIBS,               &MaxVertexAttribs);
		glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS,   &MaxVertexTextureImageUnits);
		glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS,       &MaxVertexUniformVectors);
		glGetIntegerv(GL_MAX_VIEWPORT_DIMS,                &MaxViewportDims);

		if (bSupportsComputeShader)
		{
			glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &MaxComputeWorkGroupInvocations);
			glGetIntegerv(GL_MAX_COMPUTE_SHARED_MEMORY_SIZE,     &MaxComputeSharedMemorySize);

			glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &MaxComputeWorkGroupSize.X);
			glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &MaxComputeWorkGroupSize.Y);
			glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &MaxComputeWorkGroupSize.Z);

			glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &MaxComputeWorkGroupCount.X);
			glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &MaxComputeWorkGroupCount.Y);
			glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &MaxComputeWorkGroupCount.Z);
		}
	}

}






