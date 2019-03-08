#include <Graphics/OpenGL/DeviceOpenGL.h>
#include <Graphics/OpenGL/ShaderOpenGL.h>
#include <Graphics/OpenGL/TextureOpenGL.h>
#include <Graphics/OpenGL/MeshOpenGL.h>
#include <Graphics/OpenGL/FramebufferOpenGL.h>

namespace Columbus
{

	DeviceOpenGL::DeviceOpenGL()
	{

	}

	ShaderProgram* DeviceOpenGL::CreateShaderProgram() const
	{
		return new ShaderProgramOpenGL();
	}

	Texture* DeviceOpenGL::CreateTexture() const
	{
		return new TextureOpenGL();
	}
	
	Mesh* DeviceOpenGL::CreateMesh() const
	{
		return new MeshOpenGL();
	}
	
	Framebuffer* DeviceOpenGL::createFramebuffer() const
	{
		return new FramebufferOpenGL();
	}
	
	DeviceOpenGL::~DeviceOpenGL()
	{

	}

}


