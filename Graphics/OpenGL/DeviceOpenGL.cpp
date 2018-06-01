#include <Graphics/OpenGL/DeviceOpenGL.h>
#include <Graphics/OpenGL/ShaderOpenGL.h>
#include <Graphics/OpenGL/CubemapOpenGL.h>
#include <Graphics/OpenGL/TextureOpenGL.h>
#include <Graphics/OpenGL/MeshOpenGL.h>
#include <Graphics/OpenGL/MeshInstancedOpenGL.h>
#include <Graphics/OpenGL/FramebufferOpenGL.h>

namespace Columbus
{

	DeviceOpenGL::DeviceOpenGL()
	{

	}

	ShaderStage* DeviceOpenGL::CreateShaderStage() const
	{
		return new ShaderStageOpenGL();
	}

	ShaderProgram* DeviceOpenGL::CreateShaderProgram() const
	{
		return new ShaderProgramOpenGL();
	}

	Cubemap* DeviceOpenGL::createCubemap() const
	{
		return new CubemapOpenGL();
	}
	
	Cubemap* DeviceOpenGL::createCubemap(std::string aPath) const
	{
		return new CubemapOpenGL(aPath);
	}
	
	Cubemap* DeviceOpenGL::createCubemap(std::array<std::string, 6> aPath) const
	{
		return new CubemapOpenGL(aPath);
	}
	
	Texture* DeviceOpenGL::createTexture() const
	{
		return new TextureOpenGL();
	}
	
	Texture* DeviceOpenGL::createTexture(std::string aPath, bool aSmooth) const
	{
		return new TextureOpenGL(aPath, aSmooth);
	}
	
	Texture* DeviceOpenGL::createTexture(const char* aData, const int aW, const int aH, bool aSmooth) const
	{
		return new TextureOpenGL(aData, aW, aH, aSmooth);
	}
	
	Mesh* DeviceOpenGL::createMesh() const
	{
		return new MeshOpenGL();
	}
	
	Mesh* DeviceOpenGL::createMesh(std::vector<Vertex> aVert) const
	{
		return new MeshOpenGL(aVert);
	}
	
	Mesh* DeviceOpenGL::createMesh(std::vector<Vertex> aVert, Material aMat) const
	{
		return new MeshOpenGL(aVert, aMat);
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








