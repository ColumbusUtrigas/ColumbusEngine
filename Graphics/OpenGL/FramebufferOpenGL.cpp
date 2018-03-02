/************************************************
*               FramebufferOpenGL.cpp           *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   12.02.2018                  *
*************************************************/

#include <Graphics/OpenGL/FramebufferOpenGL.h>
#include <Graphics/OpenGL/TextureOpenGL.h>
#include <GL/glew.h>

namespace Columbus
{

	C_FramebufferOpenGL::C_FramebufferOpenGL()
	{
		glGenFramebuffers(1, &mID);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	void C_FramebufferOpenGL::bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, mID);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_FramebufferOpenGL::unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	bool C_FramebufferOpenGL::setTexture2D(C_FRAMEBUFFER_ATTACHMENT aAttach, C_Texture* aTexture)
	{
		if (aTexture == nullptr) return false;
		if (aTexture->getType() != "OpenGL Texture") return false;

		unsigned int id = 0;
		id = static_cast<C_TextureOpenGL*>(aTexture)->getID();

		if (!glIsTexture(id)) return false;

		unsigned int attach = GL_COLOR_ATTACHMENT0;

		switch (aAttach)
		{
		case C_FRAMEBUFFER_COLOR_ATTACH: attach = GL_COLOR_ATTACHMENT0; break;
		case C_FRAMEBUFFER_DEPTH_ATTACH: attach = GL_DEPTH_ATTACHMENT; break;
		case C_FRAMEBUFFER_STENCIL_ATTACH: attach = GL_STENCIL_ATTACHMENT; break;
		}

		bind();
		glFramebufferTexture2D(GL_FRAMEBUFFER, attach, GL_TEXTURE_2D, id, 0);
		unbind();

		return true;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool C_FramebufferOpenGL::prepare(C_Vector4 aClear, C_Vector2 aWindowSize)
	{
		bind();
		glViewport(0, 0, aWindowSize.x, aWindowSize.y);
		glClearColor(aClear.x, aClear.y, aClear.z, aClear.w);
		glClear(C_OGL_COLOR_BUFFER_BIT | C_OGL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		return true;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool C_FramebufferOpenGL::check()
	{
		bind();
		bool ret = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		unbind();

		return ret;
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	C_FramebufferOpenGL::~C_FramebufferOpenGL()
	{
		glDeleteFramebuffers(1, &mID);
	}

}










