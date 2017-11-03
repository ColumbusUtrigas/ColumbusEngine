/************************************************
*                  Buffer.cpp                   *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*             Nikolay(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#include <RenderAPI/Buffer.h>

namespace Columbus
{

	//////////////////////////////////////////////////////////////////////////////
	//Constructor
	C_Buffer::C_Buffer()
	{
		C_GenBufferOpenGL(&mID);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Constructor 2
	C_Buffer::C_Buffer(const float* aData, const unsigned int aSize, const unsigned int aPackSize)
	{
		C_GenBufferOpenGL(&mID);
		setData(aData, aSize, aPackSize);
		compile();
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set vertices to buffer
	void C_Buffer::setData(const float* aData, const unsigned int aSize, const unsigned int aPackSize)
	{
		mData = aData;
		mSize = (unsigned int)aSize;
		mCount = aSize / sizeof(float) / aPackSize;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Load buffer data to GPU
	void C_Buffer::compile()
	{
		C_BindBufferOpenGL(C_OGL_ARRAY_BUFFER, mID);
		C_BufferDataOpenGL(C_OGL_ARRAY_BUFFER, mSize, mData, C_OGL_STATIC_DRAW);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Bind vertex buffer
	void C_Buffer::bind()
	{
		C_BindBufferOpenGL(C_OGL_ARRAY_BUFFER, mID);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Bind buffer, set vertex attribute and open vertex attribute stream
	void C_Buffer::bind(unsigned int aIndex, unsigned int aNorm, size_t aStride)
	{
		unsigned int size = mSize / sizeof(float) / mCount;

		bind();
		C_VertexAttribPointerOpenGL(aIndex, size, C_OGL_FLOAT, aNorm, aStride, NULL);
		C_OpenStreamOpenGL(aIndex);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Unbind vertex buffer
	void C_Buffer::unbind()
	{
		C_BindBufferOpenGL(C_OGL_ARRAY_BUFFER, 0);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return packs count
	int C_Buffer::getCount()
	{
		return mCount;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Destructor
	C_Buffer::~C_Buffer()
	{
		C_DeleteBufferOpenGL(&mID);
	}

}
