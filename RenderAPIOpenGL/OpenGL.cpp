#include <RenderAPIOpenGL/OpenGL.h>
#include <GL/glew.h>

namespace Columbus
{

	int32 OpenGL::MaxCubemapTextureSize = -1;
	int32 OpenGL::MaxTextureSize = -1;
	int32 OpenGL::MaxColorTextureSamples = -1;
	int32 OpenGL::MaxDepthTextureSamples = -1;
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

		glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &MaxCubemapTextureSize);
		glGetIntegerv(GL_MAX_TEXTURE_SIZE,          &MaxTextureSize);
		glGetIntegerv(GL_MAX_COLOR_TEXTURE_SAMPLES, &MaxColorTextureSamples);
		glGetIntegerv(GL_MAX_DEPTH_TEXTURE_SAMPLES, &MaxDepthTextureSamples);

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


		glEnable(GL_MULTISAMPLE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glEnable(GL_PROGRAM_POINT_SIZE);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS_ARB);
	}

}


