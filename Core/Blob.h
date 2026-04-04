#pragma once

#include <vector>

#include "Core/Types.h"
#include "Core/Reflection.h"

namespace Columbus
{

	struct Blob
	{
		std::vector<u8> Bytes;

		void Clear()
		{
			Bytes.clear();
		}

		bool Empty() const
		{
			return Bytes.empty();
		}

		u64 Size() const
		{
			return Bytes.size();
		}

		u8* Data()
		{
			return Bytes.empty() ? nullptr : Bytes.data();
		}

		const u8* Data() const
		{
			return Bytes.empty() ? nullptr : Bytes.data();
		}
	};

}

CREFLECT_DECLARE_STRUCT(Columbus::Blob, 1, "6F7357B3-20F7-4D36-92B7-9FC44EBD2A61");
