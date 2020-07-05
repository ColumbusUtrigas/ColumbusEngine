#include <Graphics/Device.h>

namespace Columbus
{

	Device* gDevice = nullptr;

	Device::Device() {}

	GraphicsAPI Device::GetCurrentAPI()
	{
		return _currentApi;
	}

	void Device::Initialize()
	{
		gDefaultShaders = new DefaultShaders();
		gDefaultTextures = new DefaultTextures();
	}

	void Device::Shutdown()
	{
		delete gDefaultShaders;
		delete gDefaultTextures;
	}

	ShaderProgram* Device::CreateShaderProgram() const
	{
		return nullptr;
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
		return nullptr;
	}
	
	Device::~Device() {}

}


