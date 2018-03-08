/************************************************
*                 PostEffect.h                  *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   20.10.2017                  *
*************************************************/

#pragma once

#include <vector>
#include <string>
#include <memory>
#include <utility>
#include <typeinfo>

#include <RenderAPI/APIOpenGL.h>
#include <RenderAPI/Buffer.h>
#include <Graphics/Cubemap.h>
#include <Graphics/Shader.h>
#include <Graphics/Framebuffer.h>
#include <Graphics/Texture.h>
#include <Graphics/Renderbuffer.h>
#include <Graphics/OpenGL/FramebufferOpenGL.h>
#include <Graphics/OpenGL/TextureOpenGL.h>

namespace Columbus
{

	struct PostEffectAttributeInt
	{
		std::string name;
		int value;
		PostEffectAttributeInt(std::string aName, int aValue) : name(aName), value(aValue) {}
	};
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	struct PostEffectAttributeFloat
	{
		std::string name;
		float value;
		PostEffectAttributeFloat(std::string aName, float aValue) : name(aName), value(aValue) {}
	};
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	struct PostEffectAttributeVector2
	{
		std::string name;
		Vector2 value;
		PostEffectAttributeVector2(std::string aName, Vector2 aValue) : name(aName), value(aValue) {}
	};
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	struct PostEffectAttributeVector3
	{
		std::string name;
		Vector3 value;
		PostEffectAttributeVector3(std::string aName, Vector3 aValue) : name(aName), value(aValue) {}
	};
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	struct PostEffectAttributeVector4
	{
		std::string name;
		Vector4 value;
		PostEffectAttributeVector4(std::string aName, Vector4 aValue) : name(aName), value(aValue) {}
	};
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	class PostEffect
	{
	private:
		Shader* mShader = nullptr;
		Framebuffer* mFB = nullptr;
		Texture* mTB = nullptr;
		Texture* mDepth = nullptr;

		std::vector<PostEffectAttributeInt> mAttribsInt;
		std::vector<PostEffectAttributeFloat> mAttribsFloat;
		std::vector<PostEffectAttributeVector2> mAttribsVector2;
		std::vector<PostEffectAttributeVector3> mAttribsVector3;
		std::vector<PostEffectAttributeVector4> mAttribsVector4;
	public:
		PostEffect();

		void setShader(Shader* aShader);
		void addAttrib(PostEffectAttributeInt aAttrib);
		void addAttrib(PostEffectAttributeFloat aAttrib);
		void addAttrib(PostEffectAttributeVector2 aAttrib);
		void addAttrib(PostEffectAttributeVector3 aAttrib);
		void addAttrib(PostEffectAttributeVector4 aAttrib);
		void clearAttribs();

		void bind(Vector4 aClear, Vector2 aWindowSize);
		void draw();
		void unbind();

		~PostEffect();
	};

}



