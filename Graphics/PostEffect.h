/************************************************
*                 PostEffect.h                  *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*             Nikolay(Columbus) Red             *
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

namespace Columbus
{

	struct C_PostEffectAttributeInt
	{
		std::string name;
		int value;
		C_PostEffectAttributeInt(std::string aName, int aValue) : name(aName), value(aValue) {}
	};
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	struct C_PostEffectAttributeFloat
	{
		std::string name;
		float value;
		C_PostEffectAttributeFloat(std::string aName, float aValue) : name(aName), value(aValue) {}
	};
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	struct C_PostEffectAttributeVector2
	{
		std::string name;
		C_Vector2 value;
		C_PostEffectAttributeVector2(std::string aName, C_Vector2 aValue) : name(aName), value(aValue) {}
	};
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	struct C_PostEffectAttributeVector3
	{
		std::string name;
		C_Vector3 value;
		C_PostEffectAttributeVector3(std::string aName, C_Vector3 aValue) : name(aName), value(aValue) {}
	};
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	struct C_PostEffectAttributeVector4
	{
		std::string name;
		C_Vector4 value;
		C_PostEffectAttributeVector4(std::string aName, C_Vector4 aValue) : name(aName), value(aValue) {}
	};
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	class C_PostEffect
	{
	private:
		C_Shader* mShader = nullptr;
		C_Framebuffer* mFB = nullptr;
		C_Texture* mTB = nullptr;
		C_Texture* mDepth = nullptr;
		C_Renderbuffer* mRB = nullptr;

		std::vector<C_PostEffectAttributeInt> mAttribsInt;
		std::vector<C_PostEffectAttributeFloat> mAttribsFloat;
		std::vector<C_PostEffectAttributeVector2> mAttribsVector2;
		std::vector<C_PostEffectAttributeVector3> mAttribsVector3;
		std::vector<C_PostEffectAttributeVector4> mAttribsVector4;
	public:
		C_PostEffect();

		void setShader(C_Shader* aShader);
		void addAttrib(C_PostEffectAttributeInt aAttrib);
		void addAttrib(C_PostEffectAttributeFloat aAttrib);
		void addAttrib(C_PostEffectAttributeVector2 aAttrib);
		void addAttrib(C_PostEffectAttributeVector3 aAttrib);
		void addAttrib(C_PostEffectAttributeVector4 aAttrib);
		void clearAttribs();
		void bind(C_Vector4 aClear, C_Vector2 aWindowSize);
		void draw();
		void unbind();

		~C_PostEffect();
	};

}
