#pragma once

#include <Graphics/Texture.h>
#include <Graphics/Renderbuffer.h>

#include <System/System.h>
#include <System/Log.h>

#include <Math/Vector2.h>
#include <Math/Vector3.h>
#include <Math/Vector4.h>

namespace Columbus
{

	class Framebuffer
	{
	public:
		enum class Attachment
		{
			Color0,
			Color1,
			Color2,
			Color3,
			Depth,
			Stencil
		};
	public:
		Framebuffer() {}

		virtual void bind() {}
		virtual void unbind() {}

		virtual bool setTexture2D(Attachment Attach, Texture* InTexture) { return false; }
		virtual bool prepare(const Vector4& Clear, const Vector2& WindowSize) { return false; }
		virtual bool check() { return false; }

		virtual ~Framebuffer() {}
	};

}
