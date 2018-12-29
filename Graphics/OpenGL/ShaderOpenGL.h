#pragma once

#include <Graphics/Shader.h>
#include <Graphics/OpenGL/TextureOpenGL.h>
#include <Math/Matrix.h>
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
		static constexpr int MaxUniforms = 64;
		int CurrentFastUniform = 0;

		std::unordered_map<std::string, uint32> UniformLocations;
		std::unordered_map<std::string, int> FastUniformsMap;

		int32 FastUniforms[MaxUniforms];

		std::vector<std::string> Uniforms;
		uint32 ID = 0;
	public:
		void* RenderData = nullptr;
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
		void SetUniformTexture(const std::string& Name, TextureOpenGL* Tex, uint32 Sampler) const;

		int GetFastUniform(const std::string& Name) const;

		void SetUniform(int FastID, int Value) const;
		void SetUniform(int FastID, float Value) const;
		void SetUniform(int FastID, const Vector2& Value) const;
		void SetUniform(int FastID, const Vector3& Value) const;
		void SetUniform(int FastID, const Vector4& Value) const;
		void SetUniform(int FastID, uint32 Size, const float* Value) const;
		void SetUniform(int FastID, bool Transpose, const Matrix& Mat) const;
		void SetUniform(int FastID, TextureOpenGL* Tex, uint32 Sampler) const;

		~ShaderProgramOpenGL() override;
	};

}



