#pragma once

#include <Graphics/Shader.h>
#include <Graphics/Texture.h>
#include <Graphics/Mesh.h>
#include <Graphics/Framebuffer.h>

namespace Columbus
{

	class Device
	{
	private:
		DefaultShaders* gDefaultShaders = nullptr;
	public:
		Device();

		void Initialize();
		void Shutdown();

		virtual ShaderProgram* CreateShaderProgram() const;
		virtual Texture* CreateTexture() const;
		virtual Mesh* CreateMesh() const;
		virtual Framebuffer* CreateFramebuffer() const;

		DefaultShaders* GetDefaultShaders() { return gDefaultShaders; }

		virtual ~Device();
	};

	extern Device* gDevice;

}


