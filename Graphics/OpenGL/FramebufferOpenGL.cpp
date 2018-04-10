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

	FramebufferOpenGL::FramebufferOpenGL()
	{
		glGenFramebuffers(1, &mID);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	void FramebufferOpenGL::bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, mID);
	}
	//////////////////////////////////////////////////////////////////////////////
	void FramebufferOpenGL::unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	bool FramebufferOpenGL::setTexture2D(FramebufferAttachment aAttach, Texture* aTexture)
	{
		if (aTexture == nullptr) return false;
		if (aTexture->getType() != "OpenGL Texture") return false;

		unsigned int id = 0;
		id = static_cast<TextureOpenGL*>(aTexture)->getID();

		if (!glIsTexture(id)) return false;

		unsigned int attach = GL_COLOR_ATTACHMENT0;

		switch (aAttach)
		{
		case E_FRAMEBUFFER_COLOR_ATTACH: attach = GL_COLOR_ATTACHMENT0; break;
		case E_FRAMEBUFFER_DEPTH_ATTACH: attach = GL_DEPTH_ATTACHMENT; break;
		case E_FRAMEBUFFER_STENCIL_ATTACH: attach = GL_STENCIL_ATTACHMENT; break;
		}

		bind();
		glFramebufferTexture2D(GL_FRAMEBUFFER, attach, GL_TEXTURE_2D, id, 0);
		unbind();

		return true;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool FramebufferOpenGL::prepare(Vector4 aClear, Vector2 aWindowSize)
	{
		bind();
		glViewport(0, 0, (uint32)(aWindowSize.X), (uint32)(aWindowSize.Y));
		glClearColor(aClear.x, aClear.y, aClear.z, aClear.w);
		glClear(C_OGL_COLOR_BUFFER_BIT | C_OGL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		return true;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool FramebufferOpenGL::check()
	{
		bind();
		bool ret = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		unbind();

		return ret;
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	FramebufferOpenGL::~FramebufferOpenGL()
	{
		glDeleteFramebuffers(1, &mID);
	}

}










