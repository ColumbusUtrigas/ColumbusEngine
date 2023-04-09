#pragma once

#include <Core/Types.h>

namespace Columbus
{

	enum class BufferType
	{
		Array,
		Index,
		Constant,
		UAV,
		AccelerationStructureStorage,
		AccelerationStructureInput,
		ShaderBindingTable
	};

	enum class BufferMapAccess
	{
		Read,
		Write,
		ReadWrite
	};

	enum class BufferUsage
	{
		Static,
		Dynamic,
		Stream
	};

	enum class BufferCpuAccess
	{
		Read,
		Write,
		Copy
	};

	struct BufferDesc
	{
		uint64 Size = 0;
		BufferUsage Usage = BufferUsage::Static;
		BufferType BindFlags = BufferType::Array;
		BufferCpuAccess CpuAccess = BufferCpuAccess::Write;
		uint32 MiscFlags;
		uint32 StructureByteStride;

		BufferDesc() {}
		BufferDesc(
			uint64 Size,
			BufferType BindFlags,
			BufferUsage Usage,
			BufferCpuAccess CpuAccess
		) :
			Size(Size),
			BindFlags(BindFlags),
			Usage(Usage),
			CpuAccess(CpuAccess) {}
	};

	class Buffer
	{
	protected:
		BufferDesc Desc;
	public:
		const BufferDesc& GetDesc() const { return Desc; }
		virtual void* GetHandle() = 0;
	};

}
