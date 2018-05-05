#pragma once

#include <Graphics/Shader.h>
#include <Core/Types.h>
#include <map>

namespace Columbus
{

	class ShaderOpenGL : public Shader
	{
	private:
		std::map<std::string, uint32> UniformLocations;
		uint32 ID = 0;
	public:
		ShaderOpenGL();

		bool Load(std::string InVert, std::string InFrag) override;
		bool Compile() override;

		void Bind() const override;
		void Unbind() const override;

		void AddUniform(std::string Name) override;
		void SetUniform1i(std::string Name, int Value) const override; 
		void SetUniform1f(std::string Name, float Value) const override;
		void SetUniform2f(std::string Name, Vector2 Value) const override;
		void SetUniform3f(std::string Name, Vector3 Value) const override;
		void SetUniform4f(std::string Name, Vector4 Value) const override;
		void SetUniformMatrix(std::string Name, const float* Value) const override;
		void SetUniformArrayf(std::string Name, const float* Array, uint32 Size) const override;

		~ShaderOpenGL() override;
	};

}



