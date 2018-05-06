#include <Graphics/Device.h>

namespace Columbus
{

	Device* gDevice = nullptr;

	
	Device::Device()
	{

	}

	ShaderStage* Device::CreateShaderStage() const
	{
		return new ShaderStage();
	}

	ShaderProgram* Device::CreateShaderProgram() const
	{
		return new ShaderProgram();
	}
	
	Cubemap* Device::createCubemap() const
	{
		return new Cubemap();
	}
	
	Cubemap* Device::createCubemap(std::string aPath) const
	{
		return new Cubemap(aPath);
	}
	
	Cubemap* Device::createCubemap(std::array<std::string, 6> aPath) const
	{
		return new Cubemap(aPath);
	}
	
	Texture* Device::createTexture() const
	{
		return new Texture();
	}
	
	Texture* Device::createTexture(std::string aPath, bool aSmooth) const
	{
		return new Texture(aPath, aSmooth);
	}
	
	Texture* Device::createTexture(const char* aData, const int aW, const int aH, bool aSmooth) const
	{
		return new Texture(aData, aW, aH, aSmooth);
	}
	
	Mesh* Device::createMesh() const
	{
		return new Mesh();
	}
	
	Mesh* Device::createMesh(std::vector<Vertex> aVert) const
	{
		return new Mesh(aVert);
	}
	
	Mesh* Device::createMesh(std::vector<Vertex> aVert, Material aMat) const
	{
		return new Mesh(aVert, aMat);
	}
	
	Framebuffer* Device::createFramebuffer() const
	{
		return new Framebuffer();
	}
	
	Device::~Device()
	{

	}

}









