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
		ShaderBuilder Builder;

		char* ShaderPath = nullptr;
		char* ShaderSource = nullptr;

		uint32 ID = 0;
	public:
		ShaderStageOpenGL();

		bool IsValid() const override;
		bool Load(const char* FileName, ShaderType InType) override;
		bool Compile() override;

		//Get OpenGL-sepcifed identifier
		uint32 GetID() const;

		~ShaderStageOpenGL() override;
	};

	class ShaderProgramOpenGL : public ShaderProgram
	{
	private:
		static constexpr int MaxAttributes = 16;
		static constexpr int MaxUniforms = 64;
		int CurrentAttribute = 0;
		int CurrentFastUniform = 0;
		int CurrentUniform = 0;

		std::unordered_map<std::string, uint32> UniformLocations;
		std::unordered_map<std::string, int> FastUniformsMap;

		ShaderAttribute Attributes[MaxAttributes];
		int32 FastUniforms[MaxUniforms];
		char* Uniforms[MaxUniforms] = { nullptr };

		uint32 ID = 0;
	public:
		void* RenderData = nullptr;
	public:
		ShaderProgramOpenGL();

		void Bind() const override;
		void Unbind() const override;

		void AddStage(ShaderStage* Stage) override;
		bool Load(const char* FileName) override;
		bool Load(StandartProgram Program) override;
		bool Compile() override;

		bool AddUniform(const char* Name) override;
		void SetUniform1i(const char* Name, int Value) const override;
		void SetUniform1f(const char* Name, float Value) const override;
		void SetUniform2f(const char* Name, const Vector2& Value) const override;
		void SetUniform3f(const char* Name, const Vector3& Value) const override;
		void SetUniform4f(const char* Name, const Vector4& Value) const override;
		void SetUniformMatrix(const char* Name, const float* Value) const override;
		void SetUniformArrayf(const char* Name, const float* Array, uint32 Size) const override;
		void SetUniformTexture(const char* Name, TextureOpenGL* Tex, uint32 Sampler) const;

		int GetFastUniform(const char* Name) const;

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



