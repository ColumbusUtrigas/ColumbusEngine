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
	"#define Position gl_Position\n\n";
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	const std::string gFragmentShaderHeader = 
	"#version 130\n"
	"#define FragColor gl_FragColor\n\n";

	ShaderBuilder::ShaderBuilder()
	{

	}
	//////////////////////////////////////////////////////////////////////////////
	ShaderBuilder::ShaderBuilder(const std::string aShader, const ShaderType aType)
	{
		build(aShader, aType);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	bool ShaderBuilder::build(const std::string aShader, const ShaderType aType)
	{
		shader.clear();

		switch (aType)
		{
		case E_SHADER_TYPE_VERTEX:
			shader = gVertexShaderHeader + "\n" + aShader + "\n";
			break;

		case E_SHADER_TYPE_FRAGMENT:
			shader = gFragmentShaderHeader + "\n" + aShader + "\n";
			break;
		};
		return false;
	}
	//////////////////////////////////////////////////////////////////////////////
	std::string ShaderBuilder::getShader() const
	{
		return shader;
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	ShaderBuilder::~ShaderBuilder()
	{

	}

}











