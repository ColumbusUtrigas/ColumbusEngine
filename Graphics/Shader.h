/************************************************
*              		  Shader.h                    *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*             Nikolay(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#pragma once

#include <GL/glew.h>
#include <cstdio>
#include <vector>

#include <Math/Vector2.h>
#include <Math/Vector3.h>
#include <Math/Vector4.h>
#include <System/System.h>

namespace C
{

	class C_Shader
	{
	private:
		GLuint mID;
		//Load shader from two files
		void load(const char* aVert, const char* aFrag);
		//Load shader from one file
		void load(const char* aFile);
	public:
		//Constructor
		C_Shader(const char* aVert, const char* aFrag);
		//Constructor 2
		C_Shader(const char* aFile);
		//Set integer uniform
		void setUniform1i(const char* aName, const int aValue);
		//Set float uniform
		void setUniform1f(const char* aName, const float aValue);
		//Set 2-axis vector uniform
		void setUniform2f(const char* aName, const C_Vector2 aValue);
		//Set 3-axis vector uniform
		void setUniform3f(const char* aName, const C_Vector3 aValue);
		//Set 4-axis vector uniform
		void setUniform4f(const char* aName, const C_Vector4 aValue);
		//Set matrix uniform
		void setUniformMatrix(const char* aName, const float* aValue);
		//Bind shader
		void bind();
		//Unbind shader
		static void unbind();
		//Destructor
		~C_Shader();
	};

}
