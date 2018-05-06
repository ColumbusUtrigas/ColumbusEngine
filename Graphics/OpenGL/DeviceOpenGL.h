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

		Texture* createTexture() const override;
		Texture* createTexture(std::string aPath, bool aSmooth = true) const override;
		Texture* createTexture(const char* aData, const int aW, const int aH, bool aSmooth = true) const override;

		Mesh* createMesh() const override;
		Mesh* createMesh(std::vector<Vertex> aVert) const override;
		Mesh* createMesh(std::vector<Vertex> aVert, Material aMat) const override;

		Framebuffer* createFramebuffer() const override;

		~DeviceOpenGL() override;
	};

}







