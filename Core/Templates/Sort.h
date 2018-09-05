#pragma once

#include <algorithm>
#include <Core/Types.h>

namespace Columbus
{

	template <typename T, typename Predicate>
	constexpr void Sort(T* First, uint32 Count, Predicate Pred)
	{
		if (First != nullptr && Count > 0)
		{
			for (uint32 i = 1; i < Count; i++)
			{
				bool Swapped = false;

				for (uint32 j = 0; j < (Count - i); j++)
				{
					if (!Pred(First[j], First[j + 1]))
					{
						std::swap(First[j], First[j + 1]);
						Swapped = true;
					}
				}

				if (!Swapped)
				{
					break;
				}
			}
		}
	}

}




















