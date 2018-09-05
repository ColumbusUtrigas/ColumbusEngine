#pragma once

#include <Core/Types.h>

namespace Columbus
{

	class Buffer
	{
	public:
		enum class Type;
		struct Properties;
	protected:
		uint64 Size;
		Type BufferType;
	public:
		enum class Type
		{
			Array
		};

		struct Properties
		{
			uint64 DataSize;
		};
	public:
		Buffer() : Size(0) {}

		virtual void Clear() {}

		virtual bool Create(Type InType, const Properties& Props) { return false; };
		virtual bool CreateArray(const Properties& Props) { return false; }

		virtual bool Load(const void* Data) { return false; }
		virtual bool Load(const Properties& Props, const void* BufferData) { return false; }

		Type GetType() const { return BufferType; }

		virtual ~Buffer() {}
	};

}

















