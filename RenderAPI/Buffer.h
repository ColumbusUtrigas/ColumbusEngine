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

#include <RenderAPI/APIOpenGL.h>
#include <vector>

namespace C
{

	class C_Buffer
	{
	private:
		unsigned int mID = 0;
		unsigned int mSize = 0;
		unsigned int mCount = 0;

		const float* mData = NULL;
	public:
		//Constructor
		C_Buffer();
		//Constructor 2
		C_Buffer(const float* aData, const unsigned int aSize);
		//Set data to buffer
		void setData(const float* aData, const unsigned aSize);
		//Load buffer data to GPU
		void compile();
		//Bind buffer
		void bind();
		//Unbind buffer
		static void unbind();
		//Return packs count
		int getCount();
		//Destructor
		~C_Buffer();
	};

}
