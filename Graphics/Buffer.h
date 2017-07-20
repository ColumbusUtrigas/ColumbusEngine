#pragma once
#include <GL/glew.h>

namespace C
{

	class C_Buffer
	{
	private:
		GLuint mID;
		int mSize;
	public:
		C_Buffer();
		C_Buffer(const float* aData, const int aSize);
		
		void setVertices(const float* aData, int aSize);
		
		void bind();
		
		static void unbind();
		
		int getSize();
		
		~C_Buffer();
	};

}













