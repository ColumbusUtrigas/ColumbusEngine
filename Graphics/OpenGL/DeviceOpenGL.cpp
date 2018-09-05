#include <Graphics/OpenGL/DeviceOpenGL.h>
#include <Graphics/OpenGL/BufferOpenGL.h>
#include <Graphics/OpenGL/ShaderOpenGL.h>
#include <Graphics/OpenGL/TextureOpenGL.h>
#include <Graphics/OpenGL/MeshOpenGL.h>
#include <Graphics/OpenGL/MeshInstancedOpenGL.h>
#include <Graphics/OpenGL/FramebufferOpenGL.h>

namespace Columbus
{

	DeviceOpenGL::DeviceOpenGL()
	{

	}

	Buffer* DeviceOpenGL::CreateBuffer() const
	{
		return new BufferOpenGL();
	}

	ShaderStage* DeviceOpenGL::CreateShaderStage() const
	{
		return new ShaderStageOpenGL();
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

	MeshInstanced* DeviceOpenGL::CreateMeshInstanced() const
	{
		return new MeshInstancedOpenGL();
	}
	
	Framebuffer* DeviceOpenGL::createFramebuffer() const
	{
		return new FramebufferOpenGL();
	}
	
	DeviceOpenGL::~DeviceOpenGL()
	{

	}

}








