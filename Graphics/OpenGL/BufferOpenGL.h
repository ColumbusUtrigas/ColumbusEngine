#pragma once

#include <Core/Types.h>
#include <GL/glew.h>
#include <vector>

namespace Columbus
{

	enum class InputLayoutSemantic
	{
		Position,
		UV,
		Normal,
		Tangent
	};

	enum class InputLayoutFormat
	{
		None = 0,
		Float,
		Float2,
		Float3,
		Float4
	};

	enum class InputLayoutClassification
	{
		PerVertex,
		PerInstance
	};

	struct InputLayoutElement
	{
		InputLayoutSemantic Semantic; // Semantic
		InputLayoutFormat Type; // Type of the element
		uint32 Index; // Index of the attribute (in DirectX that also index of the buffer slot)
		uint32 Offset; // Offset from the beginning of the current vertex
		InputLayoutClassification Classification; // PerVertex or PerInstance
	};
	
	struct InputLayout
	{
		std::vector<InputLayoutElement> Elements;

		InputLayout(const std::initializer_list<InputLayoutElement> Elements) :
			Elements(Elements) {}
	};

	class BufferOpenGL
	{
	public:
		enum class AccessType
		{
			ReadOnly,
			WriteOnly,
			ReadWrite
		};

		enum class UsageType
		{
			Read,
			Write,
			Copy
		};

		enum class FrequencyType
		{
			Static,
			Dynamic,
			Stream
		};

		enum class Type
		{
			Array,
			Index
		};

		struct Properties
		{
			uint64 Size;
			UsageType Usage;
			FrequencyType Frequency;

			Properties(uint64 Size = 0, UsageType Usage = UsageType::Write,
				FrequencyType Frequency = FrequencyType::Static) :
				Size(Size), Usage(Usage), Frequency(Frequency) {}
		};
	private:
		uint64 Size;
		uint32 ID;
		uint32 Target;
		uint32 Usage;

		Type BufferType;
	public:
		BufferOpenGL();
		BufferOpenGL(Type Type);
		BufferOpenGL(Type Type, Properties Props);
		BufferOpenGL(Type Type, Properties Props, const void* Data);
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

		void Create(Type Type, const Properties& Props);
		void CreateArray(const Properties& Props);
		void CreateIndex(const Properties& Props);

		void Load(const void* Data);
		void Load(const void* Data, const Properties& Props);

		void Bind() const;
		void Unbind() const;

		void* Map(AccessType Access) const;
		void Unmap() const;

		template <typename Type>
		void VertexAttribute(uint32 Index, uint32 Components, bool Normalized,
				uint32 Stride, const void* Pointer) const;

		~BufferOpenGL();
	};

	template <>
	inline void BufferOpenGL::VertexAttribute<float>(uint32 Index, uint32 Components,
	bool Normalized, uint32 Stride, const void* Pointer) const
	{
		glBindBuffer(Target, ID);
		glVertexAttribPointer(Index, Components, GL_FLOAT, Normalized, Stride, Pointer);
		glEnableVertexAttribArray(Index);
	}

}


