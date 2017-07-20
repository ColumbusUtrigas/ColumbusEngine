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
		void load(const char* aVert, const char* aFrag);
		void load(const char* aFile);
	public:
		C_Shader(const char* aVert, const char* aFrag);
		C_Shader(const char* aFile);
		
		void setUniform1i(const char* aName, const int aValue);
		void setUniform1f(const char* aName, const float aValue);
		void setUniform2f(const char* aName, const C_Vector2 aValue);
		void setUniform3f(const char* aName, const C_Vector3 aValue);
		void setUniform4f(const char* aName, const C_Vector4 aValue);
		void setUniformMatrix(const char* aName, const float* aValue);
		
		void bind();
		
		static void unbind();
		
		~C_Shader();
	};

}











