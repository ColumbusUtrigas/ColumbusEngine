#include <Graphics/OpenGL/FramebufferOpenGL.h>
#include <Graphics/OpenGL/TextureOpenGL.h>
#include <GL/glew.h>

namespace Columbus
{

	FramebufferOpenGL::FramebufferOpenGL()
	{
		glGenFramebuffers(1, &mID);
	}
	
	void FramebufferOpenGL::bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, mID);
	}
	
	void FramebufferOpenGL::unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	
	bool FramebufferOpenGL::setTexture2D(Framebuffer::Attachment Attach, Texture* InTexture)
	{
		if (InTexture == nullptr) return false;

		uint32 id = static_cast<TextureOpenGL*>(InTexture)->GetID();

		if (!glIsTexture(id)) return false;

		uint32 attach = GL_COLOR_ATTACHMENT0;

		switch (Attach)
		{
		case Framebuffer::Attachment::Color0:  attach = GL_COLOR_ATTACHMENT0;  break;
		case Framebuffer::Attachment::Color1:  attach = GL_COLOR_ATTACHMENT1;  break;
		case Framebuffer::Attachment::Color2:  attach = GL_COLOR_ATTACHMENT2;  break;
		case Framebuffer::Attachment::Color3:  attach = GL_COLOR_ATTACHMENT3;  break;
		case Framebuffer::Attachment::Depth:   attach = GL_DEPTH_ATTACHMENT;   break;
		case Framebuffer::Attachment::Stencil: attach = GL_STENCIL_ATTACHMENT; break;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, mID);
		glFramebufferTexture2D(GL_FRAMEBUFFER, attach, GL_TEXTURE_2D, id, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		return true;
	}
	
	bool FramebufferOpenGL::prepare(const Vector4& Clear, const Vector2& WindowSize)
	{
		bind();
		glViewport(0, 0, (uint32)(WindowSize.X), (uint32)(WindowSize.Y));
		glClearColor(Clear.X, Clear.Y, Clear.Z, Clear.W);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		return true;
	}
	
	bool FramebufferOpenGL::check()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, mID);
		int ret = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		return ret == GL_FRAMEBUFFER_COMPLETE;
	}
	
	FramebufferOpenGL::~FramebufferOpenGL()
	{
		glDeleteFramebuffers(1, &mID);
	}

}




