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

	enum E_SHADER_TYPE
	{
		E_SHADER_TYPE_VERTEX,
		E_SHADER_TYPE_FRAGMENT
	};

	class C_ShaderBuilder
	{
	public:
		std::string shader;

		C_ShaderBuilder();
		C_ShaderBuilder(const std::string aShader, const int aType);

		bool build(const std::string aShader, const int aType);
		std::string getShader() const;

		~C_ShaderBuilder();
	};

}










