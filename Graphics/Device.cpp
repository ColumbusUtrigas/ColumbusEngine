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

	MeshInstanced* Device::CreateMeshInstanced() const
	{
		return new MeshInstanced();
	}
	
	Framebuffer* Device::createFramebuffer() const
	{
		return new Framebuffer();
	}
	
	Device::~Device()
	{

	}

}









