#pragma once

#include <Graphics/Device.h>

namespace Columbus
{

	class DeviceOpenGL : public Device
	{
	public:
		DeviceOpenGL();

		ShaderStage* CreateShaderStage() const override;
		ShaderProgram* CreateShaderProgram() const override;

		Cubemap* createCubemap() const override;
		Cubemap* createCubemap(std::string aPath) const override;
		Cubemap* createCubemap(std::array<std::string, 6> aPath) const override;

		Texture* CreateTexture() const override;

		Mesh* CreateMesh() const override;
		MeshInstanced* CreateMeshInstanced() const override;

		Framebuffer* createFramebuffer() const override;

		~DeviceOpenGL() override;
	};

}







