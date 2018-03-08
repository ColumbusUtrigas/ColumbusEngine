/************************************************
*                 ShaderBuilder.h               *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   11.01.2018                  *
*************************************************/
#pragma once

#include <System/System.h>

namespace Columbus
{

	enum ShaderType
	{
		E_SHADER_TYPE_VERTEX,
		E_SHADER_TYPE_FRAGMENT
	};

	class ShaderBuilder
	{
	public:
		std::string shader;

		ShaderBuilder();
		ShaderBuilder(const std::string aShader, const ShaderType aType);

		bool build(const std::string aShader, const ShaderType aType);
		std::string getShader() const;

		~ShaderBuilder();
	};

}










