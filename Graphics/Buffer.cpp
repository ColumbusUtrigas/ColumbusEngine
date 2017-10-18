/************************************************
*                  Buffer.cpp                   *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*             Nikolay(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#include <Graphics/Buffer.h>

namespace C
{

	//////////////////////////////////////////////////////////////////////////////
	//Constructor
	C_Buffer::C_Buffer()
	{
		glGenBuffers(1, &mID);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Constructor 2
	C_Buffer::C_Buffer(const float* aData, const int aSize)
	{
		glGenBuffers(1, &mID);
		setVertices(aData, (int)aSize);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set vertices to buffer
	void C_Buffer::setVertices(const float* aData, int aSize)
	{
		glBindBuffer(GL_ARRAY_BUFFER, mID);
		glBufferData(GL_ARRAY_BUFFER, aSize, aData, GL_STATIC_DRAW);
		//glBindBuffer(GL_ARRAY_BUFFER, 0);
		mSize = aSize / sizeof(float) / 3;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Bind vertex buffer
	void C_Buffer::bind()
	{
		glBindBuffer(GL_ARRAY_BUFFER, mID);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Unbind vertex buffer
	void C_Buffer::unbind()
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return vertices count
	int C_Buffer::getSize()
	{
		return mSize;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Check buffer
	bool C_Buffer::check()
	{
		return glIsVertexArray(mID);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Destructor
	C_Buffer::~C_Buffer()
	{
		glDeleteBuffers(1, &mID);
	}

}
