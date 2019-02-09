#include <Graphics/OpenGL/FramebufferOpenGL.h>
#include <Graphics/OpenGL/TextureOpenGL.h>
#include <GL/glew.h>

namespace Columbus
{

	static GLenum ConvertAttachment(Framebuffer::Attachment Attachment)
	{
		switch (Attachment)
		{
		case Framebuffer::Attachment::Color0:  return GL_COLOR_ATTACHMENT0;  break;
		case Framebuffer::Attachment::Color1:  return GL_COLOR_ATTACHMENT1;  break;
		case Framebuffer::Attachment::Color2:  return GL_COLOR_ATTACHMENT2;  break;
		case Framebuffer::Attachment::Color3:  return GL_COLOR_ATTACHMENT3;  break;
		case Framebuffer::Attachment::Depth:   return GL_DEPTH_ATTACHMENT;   break;
		case Framebuffer::Attachment::Stencil: return GL_STENCIL_ATTACHMENT; break;
		}

		return GL_INVALID_ENUM;
	}

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
	
	bool FramebufferOpenGL::setTexture2D(Framebuffer::Attachment Attach, Texture* InTexture, uint32 Level)
	{
		if (InTexture == nullptr) return false;

		uint32 ID = ((TextureOpenGL*)InTexture)->GetID();
		uint32 Attachment = ConvertAttachment(Attach);

		if (Attachment == GL_INVALID_ENUM) return false;
		if (!glIsTexture(ID)) return false;

		glBindFramebuffer(GL_FRAMEBUFFER, mID);
		glFramebufferTexture2D(GL_FRAMEBUFFER, Attachment, GL_TEXTURE_2D, ID, Level);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		return true;
	}

	bool FramebufferOpenGL::SetTextureCube(FramebufferOpenGL::Attachment Attach, Texture* InTexture, uint32 Face, uint32 Level)
	{
		if (InTexture == nullptr) return false;

		uint32 ID = ((TextureOpenGL*)InTexture)->GetID();
		uint32 Attachment = ConvertAttachment(Attach);

		if (Attachment == GL_INVALID_ENUM) return false;
		if (!glIsTexture(ID)) return false;

		glBindFramebuffer(GL_FRAMEBUFFER, mID);
		glFramebufferTexture2D(GL_FRAMEBUFFER, Attachment, GL_TEXTURE_CUBE_MAP_POSITIVE_X + Face, ID, Level);
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




