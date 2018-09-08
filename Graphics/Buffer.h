#pragma once

#include <Core/Types.h>

namespace Columbus
{

	class Buffer
	{
	public:
		enum class Type;
		enum class Usage;
		enum class Changing;
		struct Properties;
	protected:
		uint64 Size;
		Type BufferType;
		Usage BufferUsage;
		Changing BufferChanging;
	public:
		enum class Type
		{
			Array
		};

		enum class Usage
		{
			Write,
			Read,
			Copy
		};

		enum class Changing
		{
			Static,
			Dynamic,
			Stream
		};

		struct Properties
		{
			static Properties Default() { return Properties{0, Buffer::Usage::Write, Buffer::Changing::Dynamic}; }

			uint64 DataSize;
			Usage DataUsage;
			Changing DataChanging;
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

















