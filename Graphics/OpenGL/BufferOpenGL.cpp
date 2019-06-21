#include <Graphics/OpenGL/BufferOpenGL.h>
#include <GL/glew.h>

namespace Columbus
{

	template <typename T> void foo() {}
	template <> void foo<float>() {}

	static uint32 DecodeUsage(BufferOpenGL::UsageType Usage, BufferOpenGL::FrequencyType Frequency)
	{
		switch (Usage)
		{
			case BufferOpenGL::UsageType::Write:
			{
				switch (Frequency)
				{
					case BufferOpenGL::FrequencyType::Static:  return GL_STATIC_DRAW;  break;
					case BufferOpenGL::FrequencyType::Dynamic: return GL_DYNAMIC_DRAW; break;
					case BufferOpenGL::FrequencyType::Stream:  return GL_STREAM_DRAW;  break;
				}

				break;
			}

			case BufferOpenGL::UsageType::Read:
			{
				switch (Frequency)
				{
					case BufferOpenGL::FrequencyType::Static:  return GL_STATIC_READ;  break;
					case BufferOpenGL::FrequencyType::Dynamic: return GL_DYNAMIC_READ; break;
					case BufferOpenGL::FrequencyType::Stream:  return GL_STREAM_READ;  break;
				}

				break;
			}

			case BufferOpenGL::UsageType::Copy:
			{
				switch (Frequency)
				{
					case BufferOpenGL::FrequencyType::Static:  return GL_STATIC_COPY;  break;
					case BufferOpenGL::FrequencyType::Dynamic: return GL_DYNAMIC_COPY; break;
					case BufferOpenGL::FrequencyType::Stream:  return GL_STREAM_COPY;  break;
				}

				break;
			}
		}

		return 0;
	}

	BufferOpenGL::BufferOpenGL() :
		Size(0), ID(0), Target(0), Usage(0), BufferType(BufferOpenGL::Type::Array)
	{
		glGenBuffers(1, &ID);
	}

	BufferOpenGL::BufferOpenGL(Type Type) :
		Size(0), ID(0), Target(0), Usage(0), BufferType(BufferOpenGL::Type::Array)
	{
		glGenBuffers(1, &ID);
		Create(Type, BufferOpenGL::Properties());
	}

	BufferOpenGL::BufferOpenGL(Type Type, Properties Props) :
		Size(0), ID(0), Target(0), Usage(0), BufferType(BufferOpenGL::Type::Array)
	{
		glGenBuffers(1, &ID);
		Create(Type, Props);
	}

	BufferOpenGL::BufferOpenGL(Type Type, Properties Props, const void* Data) :
		Size(0), ID(0), Target(0), Usage(0), BufferType(BufferOpenGL::Type::Array)
	{
		glGenBuffers(1, &ID);
		Create(Type, Props);
		Load(Data);
	}

	void BufferOpenGL::Create(BufferOpenGL::Type Type, const BufferOpenGL::Properties& Props)
	{
		BufferType = Type;

		switch (Type)
		{
		case BufferOpenGL::Type::Array: CreateArray(Props); break;
		case BufferOpenGL::Type::Index: CreateIndex(Props); break;
		}
	}

	void BufferOpenGL::CreateArray(const BufferOpenGL::Properties& Props)
	{
		Size = Props.Size;
		Usage = DecodeUsage(Props.Usage, Props.Frequency);
		Target = GL_ARRAY_BUFFER;
		BufferType = BufferOpenGL::Type::Array;
	}

	void BufferOpenGL::CreateIndex(const BufferOpenGL::Properties& Props)
	{
		Size = Props.Size;
		Usage = DecodeUsage(Props.Usage, Props.Frequency);
		Target = GL_ELEMENT_ARRAY_BUFFER;
		BufferType = BufferOpenGL::Type::Index;
	}

	void BufferOpenGL::Load(const void* Data)
	{
		glBindBuffer(Target, ID);
		glBufferData(Target, Size, Data, Usage);
		glBindBuffer(Target, 0);
	}

	void BufferOpenGL::Load(const void* Data, const BufferOpenGL::Properties& Props)
	{
		Size = Props.Size;
		Usage = DecodeUsage(Props.Usage, Props.Frequency);

		glBindBuffer(Target, ID);
		glBufferData(Target, Size, Data, Usage);
		glBindBuffer(Target, 0);
	}

	void BufferOpenGL::Bind() const
	{
		glBindBuffer(Target, ID);
	}

	void BufferOpenGL::Unbind() const
	{
		glBindBuffer(Target, 0);
	}

	void* BufferOpenGL::Map(BufferOpenGL::AccessType Access) const
	{
		uint32 A = 0;
		switch (Access)
		{
		case BufferOpenGL::AccessType::ReadOnly:  A = GL_READ_ONLY;  break;
		case BufferOpenGL::AccessType::WriteOnly: A = GL_WRITE_ONLY; break;
		case BufferOpenGL::AccessType::ReadWrite: A = GL_READ_WRITE; break;
		}

		glBindBuffer(Target, ID);
		void* Result = glMapBuffer(Target, A);
		glBindBuffer(Target, 0);
		return Result;
	}

	void BufferOpenGL::Unmap() const
	{
		glBindBuffer(Target, ID);
		glUnmapBuffer(Target);
		glBindBuffer(Target, 0);
	}

	BufferOpenGL::~BufferOpenGL()
	{
		glDeleteBuffers(1, &ID);
	}

}


