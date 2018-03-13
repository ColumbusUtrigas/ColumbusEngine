/************************************************
*                  Framebuffer.h                *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   05.10.2017                  *
*************************************************/

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

	enum FramebufferAttachment
	{
		E_FRAMEBUFFER_COLOR_ATTACH,
		E_FRAMEBUFFER_DEPTH_ATTACH,
		E_FRAMEBUFFER_STENCIL_ATTACH
	};

	class Framebuffer
	{
	public:
		Framebuffer();

		virtual void bind();
		virtual void unbind();

		virtual bool setTexture2D(FramebufferAttachment aAttach, Texture* aTexture);
		virtual bool prepare(Vector4 aClear, Vector2 aWindowSize);
		virtual bool check();

		virtual ~Framebuffer();
	};

}
