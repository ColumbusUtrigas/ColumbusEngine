#pragma once

#include <Math/Vector2.h>
#include <Math/Vector3.h>
#include <Math/Vector4.h>
#include <Graphics/ShaderBuilder.h>

namespace Columbus
{

	struct ShaderAttribute
	{
		char* Name = nullptr;
		uint32 Value;
	};

	class ShaderStage
	{
	protected:
		ShaderType Type;

		bool Loaded;
		bool Compiled;
		bool Error;
	public:
		ShaderStage() {}

		virtual bool IsValid() const { return false; }
		virtual bool Load(const char* FileName, ShaderType InType) { return false; }
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
		static constexpr int MaxStages = 2; // Vertex, Fragment
		int CurrentStage = 0;
		ShaderStage* Stages[MaxStages];

		bool Loaded;
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
			return Loaded;
		}

		bool IsCompiled() const
		{
			return Compiled;
		}

		bool IsError() const
		{
			return Error;
		}

		virtual void Bind() const {}
		virtual void Unbind() const {}

		virtual void AddStage(ShaderStage* Stage) {}
		virtual bool Load(const char* FileName) { return false; }
		virtual bool Load(StandartProgram Program) { return false; }
		virtual bool Compile() { return false; }

		virtual bool AddUniform(const char* Name) { return false; }
		virtual void SetUniform1i(const char* Name, int Value) const {}
		virtual void SetUniform1f(const char* Name, float Value) const {}
		virtual void SetUniform2f(const char* Name, const Vector2& Value) const {}
		virtual void SetUniform3f(const char* Name, const Vector3& Value) const {}
		virtual void SetUniform4f(const char* Name, const Vector4& Value) const {}
		virtual void SetUniformMatrix(const char* Name, const float* Value) const {}
		virtual void SetUniformArrayf(const char* Name, const float* Array, uint32 Size) const {}

		virtual ~ShaderProgram() {}
	};

}
