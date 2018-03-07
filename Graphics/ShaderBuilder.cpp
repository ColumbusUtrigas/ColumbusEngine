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

	C_ShaderBuilder::C_ShaderBuilder()
	{

	}
	//////////////////////////////////////////////////////////////////////////////
	C_ShaderBuilder::C_ShaderBuilder(const std::string aShader, const int aType)
	{
		build(aShader, aType);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	bool C_ShaderBuilder::build(const std::string aShader, const int aType)
	{
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
	std::string C_ShaderBuilder::getShader() const
	{
		return shader;
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	C_ShaderBuilder::~C_ShaderBuilder()
	{

	}

}











