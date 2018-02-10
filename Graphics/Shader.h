/************************************************
*                   Shader.h                    *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#pragma once

#include <GL/glew.h>
#include <cstdio>
#include <vector>
#include <string>

#include <System/File.h>
#include <System/System.h>
#include <System/Log.h>
#include <Math/Vector2.h>
#include <Math/Vector3.h>
#include <Math/Vector4.h>
#include <Graphics/ShaderBuilder.h>

namespace Columbus
{

	struct C_ShaderAttribute
	{
		std::string name;
		int value;

		C_ShaderAttribute(const std::string aName, const int aValue) :
			name(aName), value(aValue) {}
	};

	class C_Shader
	{
	private:
		C_ShaderBuilder mBuilder;
		unsigned int mID;
		std::vector<C_ShaderAttribute> mAttributes;

		std::string mVertShaderPath;
		std::string mFragShaderPath;

		std::string mVertShaderSource;
		std::string mFragShaderSource;

		bool mLoaded = false;
		bool mCompiled = false;

		void load(const char* aFile);
	public:
		C_Shader(std::string aVert, std::string aFrag);
		C_Shader(const char* aFile);
		C_Shader();

		bool load(std::string aVert, std::string aFrag);
		bool compile();

		bool isCompiled() const;

		void bind() const;
		static void unbind();

		void addAttribute(std::string aName, const int aValue);
		void setUniform1i(std::string aName, const int aValue) const;
		void setUniform1f(std::string aName, const float aValue) const;
		void setUniform2f(std::string aName, const C_Vector2 aValue) const;
		void setUniform3f(std::string aName, const C_Vector3 aValue) const;
		void setUniform4f(std::string aName, const C_Vector4 aValue) const;
		void setUniformMatrix(std::string aName, const float* aValue) const;
		void setUniformArrayf(std::string aName, const float aArray[], const size_t aSize) const;

		~C_Shader();
	};

}
