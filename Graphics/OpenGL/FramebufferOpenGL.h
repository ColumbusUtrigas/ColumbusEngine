#pragma once

#include <Core/Types.h>
#include <Graphics/Framebuffer.h>

namespace Columbus
{

	class FramebufferOpenGL : public Framebuffer
	{
	private:
		friend class Renderer;

		uint32 ID = 0;
	public:
		FramebufferOpenGL();

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual bool SetTexture2D(Attachment Attach, Texture* InTexture, uint32 Level = 0) override;
		virtual bool SetTextureCube(Attachment Attach, Texture* InTexture, uint32 Face, uint32 Level = 0) override;
		virtual bool Prepare(const Vector4& Color, const iVector2& Origin, const iVector2& Size) override;
		virtual bool Check() override;

		virtual ~FramebufferOpenGL() override;
	};

}


                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   