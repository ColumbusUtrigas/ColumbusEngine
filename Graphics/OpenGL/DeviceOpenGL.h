#pragma once

#include <Graphics/Device.h>

namespace Columbus
{

	class C_DeviceOpenGL : public C_Device
	{
	public:
		C_DeviceOpenGL();

		C_Shader* createShader() const override;
		C_Shader* createShader(std::string aVert, std::string aFrag) const override;

		C_Cubemap* createCubemap() const override;
		C_Cubemap* createCubemap(std::string aPath) const override;
		C_Cubemap* createCubemap(std::array<std::string, 6> aPath) const override;

		C_Texture* createTexture() const override;
		C_Texture* createTexture(std::string aPath, bool aSmooth = true) const override;
		C_Texture* createTexture(const char* aData, const int aW, const int aH, bool aSmooth = true) const override;

		C_Mesh* createMesh() const override;
		C_Mesh* createMesh(std::vector<C_Vertex> aVert) const override;
		C_Mesh* createMesh(std::vector<C_Vertex> aVert, C_Material aMat) const override;

		~C_DeviceOpenGL();
	};

}







