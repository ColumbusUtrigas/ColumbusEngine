#pragma once

namespace Columbus
{

	enum class ShaderType
	{
		Vertex,
		Fragment
	};

	class ShaderBuilder
	{
	public:
		char* ShaderSource = nullptr;
		unsigned int ShaderSourceLength = 0;
	public:
		ShaderBuilder() {}

		bool Build(const char* InShader, ShaderType Type);

		~ShaderBuilder() { delete[] ShaderSource; }
	};

}


