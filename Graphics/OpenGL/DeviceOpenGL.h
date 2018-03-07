#pragma once

#include <Graphics/Device.h>

namespace Columbus
{

	class DeviceOpenGL : public Device
	{
	public:
		DeviceOpenGL();

		Shader* createShader() const override;
		Shader* createShader(std::string aVert, std::string aFrag) const override;

		Cubemap* createCubemap() const override;
		Cubemap* createCubemap(std::string aPath) const override;
		Cubemap* createCubemap(std::array<std::string, 6> aPath) const override;

		Texture* createTexture() const override;
		Texture* createTexture(std::string aPath, bool aSmooth = true) const override;
		Texture* createTexture(const char* aData, const int aW, const int aH, bool aSmooth = true) const override;

		Mesh* createMesh() const override;
		Mesh* createMesh(std::vector<Vertex> aVert) const override;
		Mesh* createMesh(std::vector<Vertex> aVert, C_Material aMat) const override;

		~DeviceOpenGL();
	};

}







