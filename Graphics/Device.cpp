#include <Graphics/Device.h>

namespace Columbus
{

	Device* gDevice = nullptr;

	Device::Device() {}

	void Device::Initialize()
	{
		gDefaultShaders = new DefaultShaders();
	}

	void Device::Shutdown()
	{
		delete gDefaultShaders;
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
		return nullptr;
	}
	
	Framebuffer* Device::CreateFramebuffer() const
	{
		return new Framebuffer();
	}
	
	Device::~Device() {}

}


