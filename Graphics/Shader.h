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

	struct ShaderAttribute
	{
		std::string Name;
		uint32 Value;

		ShaderAttribute(std::string InName, uint32 InValue) :
			Name(InName), Value(InValue) {}
	};

	class Shader
	{
	protected:
		ShaderBuilder Builder;
		std::vector<ShaderAttribute> Attributes;

		std::string VertShaderPath;
		std::string FragShaderPath;

		std::string VertShaderSource;
		std::string FragShaderSource;

		bool Loaded = false;
		bool Compiled = false;
	public:
		Shader() : Compiled(false), Loaded(false) {}
		//Shader(std::string InVert, std::string InFrag)  : Compiled(false), Loaded(false) { Load(InVert, InFrag); }

		virtual bool Load(std::string aVert, std::string aFrag) { return false; }
		virtual bool Compile() { return false; }

		bool IsLoaded() const { return Loaded; }
		bool IsCompiled() const { return Compiled; }

		virtual void Bind() const {}
		virtual void Unbind() const {}

		void AddAttribute(std::string InName, uint32 InValue) { Attributes.emplace_back(InName, InValue); }
		virtual void SetUniform1i(std::string Name, int Value) const {}
		virtual void SetUniform1f(std::string Name, float Value) const {}
		virtual void SetUniform2f(std::string Name, Vector2 Value) const {}
		virtual void SetUniform3f(std::string Name, Vector3 Value) const {}
		virtual void SetUniform4f(std::string Name, Vector4 Value) const {}
		virtual void SetUniformMatrix(std::string Name, const float* Value) const {}
		virtual void SetUniformArrayf(std::string Name, const float* Array, uint32 Size) const {}

		virtual ~Shader() {}
	};

}
