#include <Graphics/Device.h>

namespace Columbus
{

	Device* gDevice = nullptr;

	
	Device::Device() {}

	Buffer* Device::CreateBuffer() const
	{
		return new Buffer();
	}

	ShaderStage* Device::CreateShaderStage() const
	{
		return new ShaderStage();
	}

	ShaderProgram* Device::CreateShaderProgram() const
	{
		return new ShaderProgram();
	}

	Texture* Device::CreateTexture() const
	{
		return nullptr;
	}

	Mesh* Device::CreateMesh() const
	{
		return new Mesh();
	}
	
	Framebuffer* Device::createFramebuffer() const
	{
		return new Framebuffer();
	}
	
	Device::~Device() {}

}









