#include <Graphics/OpenGL/BufferOpenGL.h>
#include <GL/glew.h>

namespace Columbus
{

	BufferOpenGL::BufferOpenGL() : Buffer::Buffer(), ID(0), Target(0), Usage(0) {}

	void BufferOpenGL::Clear()
	{
		if (glIsBuffer(ID))
		{
			glDeleteBuffers(1, &ID);
		}

		Size = 0;
		ID = 0;
		Target = 0;
		Usage = 0;
	}

	bool BufferOpenGL::Create(Buffer::Type InType, const Buffer::Properties& Props)
	{
		switch (InType)
		{
			case Buffer::Type::Array: return CreateArray(Props); break;
		}

		return false;
	}

	bool BufferOpenGL::CreateArray(const Buffer::Properties& Props)
	{
		Clear();

		Size = Props.DataSize;
		BufferType = Buffer::Type::Array;

		Target = GL_ARRAY_BUFFER;
		Usage = GL_DYNAMIC_DRAW;

		glGenBuffers(1, &ID);

		return true;
	}

	bool BufferOpenGL::Load(const void* Data)
	{
		if (glIsBuffer(ID) && Size != 0)
		{
			glBindBuffer(Target, ID);
			glBufferData(Target, Size, Data, Usage);
			glBindBuffer(Target, 0);

			return true;
		}

		return false;
	}

	bool BufferOpenGL::Load(const Buffer:: Properties& Props, const void* Data)
	{
		Size = Props.DataSize;

		if (glIsBuffer(ID) && Size != 0)
		{
			glBindBuffer(Target, ID);
			glBufferData(Target, Size, Data, Usage);
			glBindBuffer(Target, 0);

			return true;
		}

		return false;
	}

	bool BufferOpenGL::Bind() const
	{
		glBindBuffer(Target, ID);
		return true;
	}

	bool BufferOpenGL::Unbind() const
	{
		glBindBuffer(Target, 0);
		return true;
	}

	BufferOpenGL::~BufferOpenGL()
	{
		Clear();
	}

}

















