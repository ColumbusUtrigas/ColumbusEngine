#include <Graphics/OpenGL/BufferOpenGL.h>
#include <GL/glew.h>

namespace Columbus
{

	void BufferOpenGL::UpdateUsage()
	{
		switch (BufferUsage)
		{
			case Buffer::Usage::Write:
			{
				switch (BufferChanging)
				{
					case Buffer::Changing::Static:  Usage = GL_STATIC_DRAW;  break;
					case Buffer::Changing::Dynamic: Usage = GL_DYNAMIC_DRAW; break;
					case Buffer::Changing::Stream:  Usage = GL_STREAM_DRAW;  break;
				}

				break;
			}

			case Buffer::Usage::Read:
			{
				switch (BufferChanging)
				{
					case Buffer::Changing::Static:  Usage = GL_STATIC_READ;  break;
					case Buffer::Changing::Dynamic: Usage = GL_DYNAMIC_READ; break;
					case Buffer::Changing::Stream:  Usage = GL_STREAM_READ;  break;
				}

				break;
			}

			case Buffer::Usage::Copy:
			{
				switch (BufferChanging)
				{
					case Buffer::Changing::Static:  Usage = GL_STATIC_COPY;  break;
					case Buffer::Changing::Dynamic: Usage = GL_DYNAMIC_COPY; break;
					case Buffer::Changing::Stream:  Usage = GL_STREAM_COPY;  break;
				}

				break;
			}
		}
	}

	BufferOpenGL::BufferOpenGL() : Buffer::Buffer(), ID(0), Target(0), Usage(0) {}

	void BufferOpenGL::Clear()
	{
		glDeleteBuffers(1, &ID);

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
		BufferUsage = Props.DataUsage;
		BufferChanging = Props.DataChanging;

		Target = GL_ARRAY_BUFFER;

		UpdateUsage();

		glGenBuffers(1, &ID);

		return true;
	}

	bool BufferOpenGL::Load(const void* Data)
	{
		glBindBuffer(Target, ID);
		glBufferData(Target, Size, Data, Usage);
		glBindBuffer(Target, 0);

		return true;
	}

	bool BufferOpenGL::Load(const Buffer:: Properties& Props, const void* Data)
	{
		Size = Props.DataSize;
		BufferUsage = Props.DataUsage;
		BufferChanging = Props.DataChanging;

		UpdateUsage();

		glBindBuffer(Target, ID);
		glBufferData(Target, Size, Data, Usage);
		glBindBuffer(Target, 0);

		return true;
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




