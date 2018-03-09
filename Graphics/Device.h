#pragma once

#include <Graphics/Shader.h>
#include <Graphics/Cubemap.h>
#include <Graphics/Texture.h>
#include <Graphics/Mesh.h>
#include <Graphics/Framebuffer.h>

namespace Columbus
{

	class Device
	{
	public:
		Device();

		virtual Shader* createShader() const;
		virtual Shader* createShader(std::string aVert, std::string aFrag) const;

		virtual Cubemap* createCubemap() const;
		virtual Cubemap* createCubemap(std::string aPath) const;
		virtual Cubemap* createCubemap(std::array<std::string, 6> aPath) const;

		virtual Texture* createTexture() const;
		virtual Texture* createTexture(std::string aPath, bool aSmooth = true) const;
		virtual Texture* createTexture(const char* aData, const int aW, const int aH, bool aSmooth = true) const;

		virtual Mesh* createMesh() const;
		virtual Mesh* createMesh(std::vector<Vertex> aVert) const;
		virtual Mesh* createMesh(std::vector<Vertex> aVert, Material aMat) const;

		virtual Framebuffer* createFramebuffer() const;

		~Device();
	};

	extern Device* gDevice;

}










