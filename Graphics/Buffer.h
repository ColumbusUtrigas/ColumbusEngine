#pragma once

namespace Columbus
{

	enum class BufferType
	{
		Array,
		Index,
		Uniform
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
		BufferType BindFlags;
		BufferUsage Usage;
		BufferCpuAccess CpuAccess;

		BufferDesc(
			uint64 Size = 0,
			BufferUsage Usage = BufferUsage::Write,
			BufferCpuAccess CpuAccess = BufferCpuAccess::Static
		) :
			Size(Size),
			Usage(Usage),
			CpuAccess(CpuAccess) {}

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
