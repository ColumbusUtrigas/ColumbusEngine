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
		glGenFramebuffers(1, &ID);
	}
	
	void FramebufferOpenGL::Bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, ID);
	}
	
	void FramebufferOpenGL::Unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	
	bool FramebufferOpenGL::SetTexture2D(Framebuffer::Attachment Attach, Texture* InTexture, uint32 Level)
	{
		if (InTexture == nullptr) return false;

		uint32 TextureID = ((TextureOpenGL*)InTexture)->GetID();
		uint32 Attachment = ConvertAttachment(Attach);

		if (Attachment == GL_INVALID_ENUM) return false;
		if (!glIsTexture(TextureID)) return false;

		glBindFramebuffer(GL_FRAMEBUFFER, ID);
		glFramebufferTexture2D(GL_FRAMEBUFFER, Attachment, GL_TEXTURE_2D, TextureID, Level);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		return true;
	}

	bool FramebufferOpenGL::SetTextureCube(FramebufferOpenGL::Attachment Attach, Texture* InTexture, uint32 Face, uint32 Level)
	{
		if (InTexture == nullptr) return false;

		uint32 TextureID = ((TextureOpenGL*)InTexture)->GetID();
		uint32 Attachment = ConvertAttachment(Attach);

		if (Attachment == GL_INVALID_ENUM) return false;
		if (!glIsTexture(TextureID)) return false;

		glBindFramebuffer(GL_FRAMEBUFFER, ID);
		glFramebufferTexture2D(GL_FRAMEBUFFER, Attachment, GL_TEXTURE_CUBE_MAP_POSITIVE_X + Face, TextureID, Level);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		return true;
	}
	
	bool FramebufferOpenGL::Prepare(const Vector4& Color, const iVector2& Origin, const iVector2& Size)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, ID);
		glViewport(Origin.X, Origin.Y, Size.X, Size.Y);
		glClearColor(Color.X, Color.Y, Color.Z, Color.W);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		return true;
	}
	
	bool FramebufferOpenGL::Check()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, ID);
		int ret = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		return ret == GL_FRAMEBUFFER_COMPLETE;
	}
	
	FramebufferOpenGL::~FramebufferOpenGL()
	{
		glDeleteFramebuffers(1, &ID);
	}

}


