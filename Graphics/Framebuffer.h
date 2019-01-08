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

		virtual void bind() {}
		virtual void unbind() {}

		virtual bool setTexture2D(Attachment Attach, Texture* InTexture) { return false; }
		virtual bool SetTextureCube(Attachment Attach, Texture* InTexture, uint32 Face) { return false; };
		virtual bool prepare(const Vector4& Clear, const Vector2& WindowSize) { return false; }
		virtual bool check() { return false; }

		virtual ~Framebuffer() {}
	};

}
