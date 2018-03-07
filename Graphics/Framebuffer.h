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

	enum C_FRAMEBUFFER_ATTACHMENT
	{
		C_FRAMEBUFFER_COLOR_ATTACH,
		C_FRAMEBUFFER_DEPTH_ATTACH,
		C_FRAMEBUFFER_STENCIL_ATTACH
	};

	class C_Framebuffer
	{
	public:
		C_Framebuffer();

		virtual void bind();
		virtual void unbind();

		virtual bool setTexture2D(C_FRAMEBUFFER_ATTACHMENT aAttach, Texture* aTexture);
		virtual bool prepare(Vector4 aClear, Vector2 aWindowSize);
		virtual bool check();

		~C_Framebuffer();
	};

}
