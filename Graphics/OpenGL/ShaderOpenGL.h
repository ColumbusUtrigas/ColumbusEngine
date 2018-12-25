#pragma once

#include <Graphics/Shader.h>
#include <Core/Types.h>
#include <unordered_map>

namespace Columbus
{

	class ShaderStageOpenGL : public ShaderStage
	{
	private:
		uint32 ID = 0;
	public:
		ShaderStageOpenGL();

		bool IsValid() const override;
		bool Load(std::string InPath, ShaderType InType) override;
		bool Compile() override;

		//Get OpenGL-sepcifed identifier
		uint32 GetID() const;

		~ShaderStageOpenGL() override;
	};

	class ShaderProgramOpenGL : public ShaderProgram
	{
	private:
		static constexpr int MaxUniforms = 256;

		mutable std::unordered_map<std::string, uint32> UniformLocations;
		mutable int32 FastUniforms[MaxUniforms]; //Uniforms ID by FastID
		std::vector<std::string> Uniforms;
		uint32 ID = 0;
	public:
		ShaderProgramOpenGL();

		void Bind() const override;
		void Unbind() const override;

		void AddStage(ShaderStage* Stage) override;
		bool Load(const std::string& FileName) override;
		bool Load(StandartProgram Program) override;
		bool Compile() override;

		bool AddUniform(const std::string& Name) override;
		void SetUniform1i(const std::string& Name, int Value) const override;
		void SetUniform1f(const std::string& Name, float Value) const override;
		void SetUniform2f(const std::string& Name, const Vector2& Value) const override;
		void SetUniform3f(const std::string& Name, const Vector3& Value) const override;
		void SetUniform4f(const std::string& Name, const Vector4& Value) const override;
		void SetUniformMatrix(const std::string& Name, const float* Value) const override;
		void SetUniformArrayf(const std::string& Name, const float* Array, uint32 Size) const override;

		~ShaderProgramOpenGL() override;
	};

}



