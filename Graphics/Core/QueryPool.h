#pragma once

#include <Core/Types.h>

namespace Columbus
{

	enum class QueryPoolType
	{
		Timestamp,
	};

	struct QueryPoolDesc
	{
		QueryPoolType Type;
		u32 Count;
	};

	class QueryPool
	{
	protected:
		QueryPoolDesc Desc;

		QueryPool(const QueryPoolDesc& Desc) : Desc(Desc) {}
	public:
		QueryPoolDesc GetDesc() const { return Desc; }
	};

}
