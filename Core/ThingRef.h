#pragma once

#include "Core/Guid.h"

namespace Columbus
{
	template <typename T>
	struct ThingRef
	{
		HGuid Guid;
		T* Thing = nullptr;

		ThingRef()
		{
			Guid = 0;
		}

		operator bool() const { return Thing != nullptr; }
		T* operator->() { return Thing; }
	};
}
