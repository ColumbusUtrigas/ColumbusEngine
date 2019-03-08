#pragma once

#include <Graphics/Shader.h>
#include <Graphics/Texture.h>
#include <Graphics/Mesh.h>
#include <Graphics/Framebuffer.h>

namespace Columbus
{

	class Device
	{
	public:
		Device();

		virtual ShaderProgram* CreateShaderProgram() const;
		virtual Texture* CreateTexture() const;
		virtual Mesh* CreateMesh() const;
		virtual Framebuffer* createFramebuffer() const;

		virtual ~Device();
	};

	extern Device* gDevice;

}


