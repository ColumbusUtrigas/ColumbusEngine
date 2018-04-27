#pragma once

#include <System/System.h>

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

		ShaderBuilder() {}

		bool Build(std::string InShader, ShaderType Type);

		~ShaderBuilder() {}
	};

}










