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

	class C_FramebufferOpenGL : public C_Framebuffer
	{
	private:
		unsigned int mID = 0;
	public:
		C_FramebufferOpenGL();

		void bind() override;
		void unbind() override;

		bool setTexture2D(C_FRAMEBUFFER_ATTACHMENT aAttach, C_Texture* aTexture) override;
		bool prepare(C_Vector4 aClear, C_Vector2 aWindowSize) override;
		bool check() override;

		~C_FramebufferOpenGL();
	};

}





