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
#include <Core/Containers/Array.h>

namespace Columbus
{

	struct ShaderAttribute
	{
		std::string Name;
		uint32 Value;

		ShaderAttribute(std::string InName, uint32 InValue) :
			Name(InName), Value(InValue) {}
	};

	class ShaderStage
	{
	protected:
		ShaderBuilder Builder;
		ShaderType Type;

		std::string ShaderPath;
		std::string ShaderSource;

		bool Loaded;
		bool Compiled;
		bool Error;
	public:
		ShaderStage() {}

		virtual bool IsValid() const { return false; }
		virtual bool Load(std::string InPath, ShaderType InType) { return false; }
		virtual bool Compile() { return false; }

		bool IsLoaded() const { return Loaded; }
		bool IsCompiled() const  { return Compiled; }
		bool IsError() const { return Error; }
		ShaderType GetType() const { return Type; }

		virtual ~ShaderStage() {}
	};

	class ShaderProgram
	{
	protected:
		std::vector<ShaderAttribute> Attributes;
		std::vector<ShaderStage*> Stages;

		bool Compiled;
		bool Error;
	public:
		enum class StandartProgram
		{
			Skybox
		};
	public:
		ShaderProgram() {}

		bool IsLoaded() const
		{
			for (auto& Stage : Stages)
			{
				if (!Stage->IsLoaded())
				{
					return false;
				}
			}

			return true;
		}

		bool IsCompiled() const
		{
			return Compiled;
			/*if (!Compiled) return false;

			for (auto& Stage : Stages)
			{
				if (!Stage->IsCompiled())
				{
					return false;
				}
			}

			return true;*/
		}

		bool IsError() const
		{
			return Error;
			/*if (Error) return true;

			for (auto& Stage : Stages)
			{
				if (Stage->IsError())
				{
					return true;
				}
			}

			return false;*/
		}

		void AddAttribute(const std::string& InName, uint32 InValue) { Attributes.emplace_back(InName, InValue); }

		virtual void Bind() const {}
		virtual void Unbind() const {}

		virtual void AddStage(ShaderStage* Stage) {}
		virtual bool Load(const std::string& FileName) { return false; }
		virtual bool Load(StandartProgram Program) { return false; }
		virtual bool Compile() { return false; }

		virtual bool AddUniform(const std::string& Name) { return false; }
		virtual void SetUniform1i(const std::string& Name, int Value) const {}
		virtual void SetUniform1f(const std::string& Name, float Value) const {}
		virtual void SetUniform2f(const std::string& Name, const Vector2& Value) const {}
		virtual void SetUniform3f(const std::string& Name, const Vector3& Value) const {}
		virtual void SetUniform4f(const std::string& Name, const Vector4& Value) const {}
		virtual void SetUniformMatrix(const std::string& Name, const float* Value) const {}
		virtual void SetUniformArrayf(const std::string& Name, const float* Array, uint32 Size) const {}

		virtual ~ShaderProgram() {}
	};

}
