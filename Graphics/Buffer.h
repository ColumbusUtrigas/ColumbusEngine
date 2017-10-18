/************************************************
*                   Buffer.h                    *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*             Nikolay(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#pragma once

#include <GL/glew.h>

namespace C
{

	class C_Buffer
	{
	private:
		unsigned int mID;
		int mSize;
	public:
		//Constructor
		C_Buffer();
		//Constructor 2
		C_Buffer(const float* aData, const int aSize);
		//Set vertices to buffer
		void setVertices(const float* aData, int aSize);
		//Bind vertex buffer
		void bind();
		//Unbind vertex buffer
		static void unbind();
		//Return vertices count
		int getSize();
		//Check buffer
		bool check();
		//Destructor
		~C_Buffer();
	};

}
