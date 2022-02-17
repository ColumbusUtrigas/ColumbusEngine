#include <Graphics/Device.h>

namespace Columbus
{

	Device* gDevice = nullptr;

	Device::Device() {}

	GraphicsAPI Device::GetCurrentAPI()
	{
		return _currentApi;
	}

	ShaderLanguage Device::GetCslBackendLang()
	{
		switch (_currentApi)
		{
		case GraphicsAPI::OpenGL:
			return ShaderLanguage::GLSL;
		case GraphicsAPI::DX12:
			return ShaderLanguage::HLSL;
		default:
			return ShaderLanguage::Undefined;
		}
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

	SPtr<Buffer> Device::CreateBufferShared(const BufferDesc& desc, SubresourceData* pInitialData)
	{
		Buffer* buf;
		CreateBuffer(desc, pInitialData, &buf);
		return SPtr<Buffer>(buf);
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


