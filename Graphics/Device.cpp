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









