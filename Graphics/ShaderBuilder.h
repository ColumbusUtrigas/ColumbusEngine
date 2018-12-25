#pragma once

#include <string>

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
		std::string ShaderSource;
	public:
		ShaderBuilder() {}

		bool Build(const std::string& InShader, ShaderType Type);

		~ShaderBuilder() {}
	};

}










