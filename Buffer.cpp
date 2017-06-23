#include "Buffer.h"

namespace C
{

	C_Buffer::C_Buffer()
	{
		glGenBuffers(1, &mID);
	}
	
	C_Buffer::C_Buffer(const float* aData, const int aSize)
	{
		glGenBuffers(1, &mID);
		setVertices(aData, (int)aSize);
	}
	
	void C_Buffer::setVertices(const float* aData, int aSize)
	{
		glBindBuffer(GL_ARRAY_BUFFER, mID);
		glBufferData(GL_ARRAY_BUFFER, aSize, aData, GL_STATIC_DRAW);
		//glBindBuffer(GL_ARRAY_BUFFER, 0);
		mSize = aSize / sizeof(float) / 3;
	}
	
	void C_Buffer::bind()
	{
		glBindBuffer(GL_ARRAY_BUFFER, mID);
	}
	
	void C_Buffer::unbind()
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	
	int C_Buffer::getSize()
	{
		return mSize;
	}
	
	C_Buffer::~C_Buffer()
	{
		glDeleteBuffers(1, &mID);
	}

}






