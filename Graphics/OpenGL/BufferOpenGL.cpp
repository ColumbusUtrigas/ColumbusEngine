#include <Graphics/OpenGL/BufferOpenGL.h>
#include <GL/glew.h>

namespace Columbus
{

	static uint32 DecodeUsage(BufferUsage Usage, BufferCpuAccess CpuAccess)
	{
		switch (Usage)
		{
			case BufferUsage::Write:
			{
				switch (CpuAccess)
				{
					case BufferCpuAccess::Static:  return GL_STATIC_DRAW;  break;
					case BufferCpuAccess::Dynamic: return GL_DYNAMIC_DRAW; break;
					case BufferCpuAccess::Stream:  return GL_STREAM_DRAW;  break;
				}

				break;
			}

			case BufferUsage::Read:
			{
				switch (CpuAccess)
				{
					case BufferCpuAccess::Static:  return GL_STATIC_READ;  break;
					case BufferCpuAccess::Dynamic: return GL_DYNAMIC_READ; break;
					case BufferCpuAccess::Stream:  return GL_STREAM_READ;  break;
				}

				break;
			}

			case BufferUsage::Copy:
			{
				switch (CpuAccess)
				{
					case BufferCpuAccess::Static:  return GL_STATIC_COPY;  break;
					case BufferCpuAccess::Dynamic: return GL_DYNAMIC_COPY; break;
					case BufferCpuAccess::Stream:  return GL_STREAM_COPY;  break;
				}

				break;
			}
		}

		return 0;
	}

	BufferOpenGL::BufferOpenGL() :
		Size(0), ID(0), Target(0), Usage(0), Type(BufferType::Array)
	{
		glGenBuffers(1, &ID);
	}

	BufferOpenGL::BufferOpenGL(BufferType NewType) :
		Size(0), ID(0), Target(0), Usage(0), Type(BufferType::Array)
	{
		glGenBuffers(1, &ID);
		Create(NewType, BufferDesc());
	}

	BufferOpenGL::BufferOpenGL(BufferType NewType, BufferDesc Desc) :
		Size(0), ID(0), Target(0), Usage(0), Type(BufferType::Array)
	{
		glGenBuffers(1, &ID);
		Create(NewType, Desc);
	}

	BufferOpenGL::BufferOpenGL(BufferType NewType, BufferDesc Desc, const void* Data) :
		Size(0), ID(0), Target(0), Usage(0), Type(BufferType::Array)
	{
		glGenBuffers(1, &ID);
		Create(NewType, Desc);
		Load(Data);
	}

	void BufferOpenGL::Create(BufferType NewType, const BufferDesc& Desc)
	{
		Type = NewType;

		switch (Type)
		{
		case BufferType::Array:   CreateArray(Desc);   break;
		case BufferType::Index:   CreateIndex(Desc);   break;
		case BufferType::Uniform: CreateUniform(Desc); break;
		}
	}

	void BufferOpenGL::CreateArray(const BufferDesc& Desc)
	{
		Size = Desc.Size;
		Usage = DecodeUsage(Desc.Usage, Desc.CpuAccess);
		Target = GL_ARRAY_BUFFER;
		Type = BufferType::Array;
		Load(nullptr);
	}

	void BufferOpenGL::CreateIndex(const BufferDesc& Desc)
	{
		Size = Desc.Size;
		Usage = DecodeUsage(Desc.Usage, Desc.CpuAccess);
		Target = GL_ELEMENT_ARRAY_BUFFER;
		Type = BufferType::Index;
		Load(nullptr);
	}

	void BufferOpenGL::CreateUniform(const BufferDesc& Desc)
	{
		Size = Desc.Size;
		Usage = DecodeUsage(Desc.Usage, Desc.CpuAccess);
		Target = GL_UNIFORM_BUFFER;
		Type = BufferType::Uniform;
		Load(nullptr);
	}

	void BufferOpenGL::Load(const void* Data)
	{
		glBindBuffer(Target, ID);
		glBufferData(Target, (GLsizeiptr)Size, Data, Usage);
		glBindBuffer(Target, 0);
	}

	void BufferOpenGL::SubLoad(const void* Data, uint32 SubdataSize, uint32 Offset)
	{
		glBindBuffer(Target, ID);
		glBufferSubData(Target, SubdataSize, Offset, Data);
		glBindBuffer(Target, 0);
	}

	void BufferOpenGL::Bind() const
	{
		glBindBuffer(Target, ID);
	}

	void BufferOpenGL::BindBase(uint32 Index) const
	{
		glBindBufferBase(Target, Index, ID);
	}

	void BufferOpenGL::BindRange(uint32 Index, uint32 Offset, uint32 Size) const
	{
		glBindBufferRange(Target, Index, ID, Offset, Size);
	}

	void BufferOpenGL::Unbind() const
	{
		glBindBuffer(Target, 0);
	}

	void BufferOpenGL::Map(void*& Dst, BufferMapAccess Access) const
	{
		Dst = Map(Access);
	}

	void* BufferOpenGL::Map(BufferMapAccess Access) const
	{
		uint32 A = 0;
		switch (Access)
		{
		case BufferMapAccess::Read:      A = GL_READ_ONLY;  break;
		case BufferMapAccess::Write:     A = GL_WRITE_ONLY; break;
		case BufferMapAccess::ReadWrite: A = GL_READ_WRITE; break;
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


