#pragma once

#include <Graphics/Shader.h>
#include <Graphics/Cubemap.h>
#include <Graphics/Texture.h>
#include <Graphics/Mesh.h>

namespace Columbus
{

	class C_Device
	{
	public:
		C_Device();

		virtual C_Shader* createShader() const;
		virtual C_Shader* createShader(std::string aVert, std::string aFrag) const;

		virtual C_Cubemap* createCubemap() const;
		virtual C_Cubemap* createCubemap(std::string aPath) const;
		virtual C_Cubemap* createCubemap(std::array<std::string, 6> aPath) const;

		virtual C_Texture* createTexture() const;
		virtual C_Texture* createTexture(std::string aPath, bool aSmooth = true) const;
		virtual C_Texture* createTexture(const char* aData, const int aW, const int aH, bool aSmooth = true) const;

		virtual C_Mesh* createMesh() const;
		virtual C_Mesh* createMesh(std::vector<C_Vertex> aVert) const;
		virtual C_Mesh* createMesh(std::vector<C_Vertex> aVert, C_Material aMat) const;

		~C_Device();
	};

	extern C_Device* gDevice;

}










