/************************************************
*                ShaderBuilder.cpp              *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   11.01.2018                  *
*************************************************/
#include <Graphics/ShaderBuilder.h>

namespace Columbus
{

	const std::string gVertexShaderHeader =
	"#version 130\n"
	"#define Position gl_Position\n";
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	const std::string gFragmentShaderHeader =
	"#version 130\n"
	"out vec4 FragColor;\n";
	
	bool ShaderBuilder::Build(std::string InShader, ShaderType Type)
	{
		ShaderSource.clear();

		switch (Type)
		{
		case ShaderType::Vertex:
			ShaderSource = gVertexShaderHeader + "\n" + InShader + "\n";
			return true;
			break;

		case ShaderType::Fragment:
			ShaderSource = gFragmentShaderHeader + "\n" + InShader + "\n";
			return true;
			break;
		};

		return false;
	}

}











