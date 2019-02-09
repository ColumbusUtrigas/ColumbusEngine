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

		virtual void bind() override;
		virtual void unbind() override;

		virtual bool setTexture2D(Attachment Attach, Texture* InTexture, uint32 Level = 0) override;
		virtual bool SetTextureCube(Attachment Attach, Texture* InTexture, uint32 Face, uint32 Level = 0) override;
		virtual bool prepare(const Vector4& aClear, const Vector2& WindowSize) override;
		virtual bool check() override;

		virtual ~FramebufferOpenGL() override;
	};

}



                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   