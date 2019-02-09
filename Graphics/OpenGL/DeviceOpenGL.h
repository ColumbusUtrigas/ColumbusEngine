#pragma once

#include <Graphics/Device.h>

namespace Columbus
{

	class DeviceOpenGL : public Device
	{
	public:
		DeviceOpenGL();

		virtual Buffer* CreateBuffer() const override;

		virtual ShaderStage* CreateShaderStage() const override;
		virtual ShaderProgram* CreateShaderProgram() const override;

		virtual Texture* CreateTexture() const override;

		virtual Mesh* CreateMesh() const override;

		virtual Framebuffer* createFramebuffer() const override;

		virtual ~DeviceOpenGL() override;
	};

}







