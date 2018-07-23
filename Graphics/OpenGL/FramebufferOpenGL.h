/************************************************
*               FramebufferOpenGL.h             *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   12.02.2018                  *
*************************************************/
#pragma once

#include <Graphics/Framebuffer.h>

namespace Columbus
{

	class FramebufferOpenGL : public Framebuffer
	{
	private:
		unsigned int mID = 0;
	public:
		FramebufferOpenGL();

		void bind() override;
		void unbind() override;

		bool setTexture2D(FramebufferAttachment aAttach, Texture* aTexture) override;
		bool prepare(Vector4 aClear, Vector2 aWindowSize) override;
		bool check() override;

		~FramebufferOpenGL() override;
	};

}





