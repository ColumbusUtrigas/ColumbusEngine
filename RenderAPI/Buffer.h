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

namespace Columbus
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
		C_Buffer(const float* aData, const unsigned int aSize, const unsigned int aPackSize);
		//Set data to buffer
		void setData(const float* aData, const unsigned aSize, const unsigned int aPackSize);
		//Load buffer data to GPU
		void compile();
		//Bind buffer
		void bind();
		//Bind buffer, set vertex attribute and open vertex attribute stream
		void bind(unsigned int aIndex, unsigned int aNorm, size_t aStride);
		//Unbind buffer
		static void unbind();
		//Return packs count
		int getCount();
		//Destructor
		~C_Buffer();
	};

}
