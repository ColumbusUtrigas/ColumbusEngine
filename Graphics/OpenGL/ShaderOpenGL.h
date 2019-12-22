#pragma once

#include <Graphics/Shader.h>
#include <Graphics/OpenGL/TextureOpenGL.h>
#include <Math/Matrix.h>
#include <Core/Types.h>
#include <unordered_map>
#include <vector>

namespace Columbus
{

	class ShaderProgramOpenGL : public ShaderProgram
	{
	private:
		struct Attribute
		{
			std::string Name;
			uint32 Slot;

			Attribute(std::string&& Name, uint32 Slot) : Name(std::move(Name)), Slot(Slot) {}
		};

		struct ParseError
		{
			size_t Line;
			size_t Position;
			std::string Message;

			ParseError(size_t Line, size_t Position, std::string&& Message) :
				Line(Line), Position(Position), Message(std::move(Message)) {}
		};

		struct ShaderData
		{
			std::string NoneSource;
			std::string VertexSource;
			std::string FragmentSource;

			std::vector<ShaderProperty> Uniforms;
			std::vector<Attribute> Attributes;

			std::vector<ParseError> Errors;
		};

		friend ShaderData ParseShader(const char* FileName);
	private:
		static constexpr int MaxUniforms = 64;
		int CurrentFastUniform = 0;

		int32 VertexShader = 0;
		int32 FragmentShader = 0;

		ShaderData Data;
		std::string Path;

		std::unordered_map<std::string, int> FastUniformsMap;

		int32 FastUniforms[MaxUniforms];

		uint32 ID = 0;
	private:
		uint32 _GetUniformID(const std::string& Name);
	public:
		void* RenderData = nullptr;
	public:
		ShaderProgramOpenGL();

		void Bind() const;
		void Unbind() const;

		int32 _GetID() const { return ID; }

		bool LoadFromMemory(const char* Source, const char* FilePath = "") override;
		bool Load(const char* FileName) override;
		bool Compile() override;

		bool AddUniform(const std::string& Name);
		int GetFastUniform(const std::string& Name) const;

		bool SetUniform(const std::string& Name, int Value) const;
		bool SetUniform(const std::string& Name, float Value) const;
		bool SetUniform(const std::string& Name, const Vector2& Value) const;
		bool SetUniform(const std::string& Name, const Vector3& Value) const;
		bool SetUniform(const std::string& Name, const Vector4& Value) const;
		bool SetUniform(const std::string& Name, uint32 Count, const float* Value) const;
		bool SetUniform(const std::string& Name, uint32 Count, const Vector2* Value) const;
		bool SetUniform(const std::string& Name, uint32 Count, const Vector3* Value) const;
		bool SetUniform(const std::string& Name, uint32 Count, const Vector4* Value) const;
		bool SetUniform(const std::string& Name, bool Transpose, const Matrix& Mat) const;
		bool SetUniform(const std::string& Name, Texture* Tex, uint32 Sampler) const;

		void SetUniform(int FastID, int Value) const;
		void SetUniform(int FastID, float Value) const;
		void SetUniform(int FastID, const Vector2& Value) const;
		void SetUniform(int FastID, const Vector3& Value) const;
		void SetUniform(int FastID, const Vector4& Value) const;
		void SetUniform(int FastID, uint32 Count, const float* Value) const;
		void SetUniform(int FastID, uint32 Count, const Vector2* Value) const;
		void SetUniform(int FastID, uint32 Count, const Vector3* Value) const;
		void SetUniform(int FastID, uint32 Count, const Vector4* Value) const;
		void SetUniform(int FastID, bool Transpose, const Matrix& Mat) const;
		void SetUniform(int FastID, TextureOpenGL* Tex, uint32 Sampler) const;

		~ShaderProgramOpenGL() override;
	};

}


