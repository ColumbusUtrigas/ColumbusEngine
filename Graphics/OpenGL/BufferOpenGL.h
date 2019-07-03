#pragma once

#include <Core/Types.h>
#include <GL/glew.h>
#include <vector>

namespace Columbus
{

	enum class BufferType
	{
		Array,
		Index
	};

	enum class BufferMapAccess
	{
		Read,
		Write,
		ReadWrite
	};

	enum class BufferUsage
	{
		Read,
		Write,
		Copy
	};

	enum class BufferCpuAccess
	{
		Static,
		Dynamic,
		Stream
	};

	struct BufferDesc
	{
		uint64 Size;
		BufferUsage Usage;
		BufferCpuAccess CpuAccess;

		BufferDesc(uint64 Size = 0, BufferUsage Usage = BufferUsage::Write,
			BufferCpuAccess CpuAccess = BufferCpuAccess::Static)
		: Size(Size), Usage(Usage), CpuAccess(CpuAccess) {}
	};

	class BufferOpenGL
	{
	private:
		uint64 Size;
		uint32 ID;
		uint32 Target;
		uint32 Usage;

		BufferType Type;
	public:
		BufferOpenGL();
		BufferOpenGL(BufferType NewType);
		BufferOpenGL(BufferType NewType, BufferDesc Desc);
		BufferOpenGL(BufferType NewType, BufferDesc Desc, const void* Data);
		BufferOpenGL(const BufferOpenGL&) = delete;
		BufferOpenGL(BufferOpenGL&& Base) { *this = static_cast<BufferOpenGL&&>(Base); }
		BufferOpenGL& operator=(const BufferOpenGL&) = delete;
		BufferOpenGL& operator=(BufferOpenGL&& Other)
		{
			ID = Other.ID;         Other.ID = 0;
			Target = Other.Target; Other.Target = 0;
			Usage = Other.Usage;   Other.Usage = 0;

			return *this;
		}

		void Create(BufferType NewType, const BufferDesc& Desc);
		void CreateArray(const BufferDesc& Desc);
		void CreateIndex(const BufferDesc& Desc);

		void Load(const void* Data);
		void SubLoad(const void* Data, uint32 SubdataSize, uint32 Offset);

		void Bind() const;
		void Unbind() const;

		void  Map(void*& Dst, BufferMapAccess Access) const;
		void* Map(BufferMapAccess Access) const;
		void Unmap() const;

		template <typename T>
		void VertexAttribute(uint32 Index, uint32 Components, bool Normalized,
				uint32 Stride, uint32 Offset) const;

		~BufferOpenGL();
	};

	template <>
	inline void BufferOpenGL::VertexAttribute<float>(uint32 Index, uint32 Components,
			bool Normalized, uint32 Stride, uint32 Offset) const
	{
		glBindBuffer(Target, ID);
		glVertexAttribPointer(Index, Components, GL_FLOAT, Normalized,
			Stride, (void*)(uintptr_t)Offset);
		glEnableVertexAttribArray(Index);
	}

}


