#include <Graphics/Device.h>

namespace Columbus
{

	Device* gDevice = nullptr;

	Device::Device() {}

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
		return nullptr;
	}
	
	Framebuffer* Device::CreateFramebuffer() const
	{
		return new Framebuffer();
	}
	
	Device::~Device() {}

}









