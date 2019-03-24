#pragma once

#include <Graphics/Texture.h>
#include <Math/Vector2.h>
#include <Math/Vector4.h>

namespace Columbus
{

	class Framebuffer
	{
	public:
		enum class Attachment
		{
			Color0  = 0,
			Color1  = 1,
			Color2  = 2,
			Color3  = 3,
			Depth   = 4,
			Stencil = 5
		};
	public:
		Framebuffer() {}

		virtual void Bind() {}
		virtual void Unbind() {}

		virtual bool SetTexture2D(Attachment Attach, Texture* InTexture, uint32 Level = 0) { return false; }
		virtual bool SetTextureCube(Attachment Attach, Texture* InTexture, uint32 Face, uint32 Level = 0) { return false; };
		virtual bool Prepare(const Vector4& Color, const iVector2& Origin, const iVector2& Size) { return false; }
		virtual bool Check() { return false; }

		virtual ~Framebuffer() {}
	};

}
