#pragma once

#include <Core/Types.h>

namespace Columbus
{

	template <typename T, uint32 N = 0>
	struct ArrayCount
	{
		static const uint32 Value = 0;
	};

	template <typename T>
	struct ArrayCount<T[], 0>
	{
		static const uint32 Value = 0;
	};

	template <typename T, uint32 N>
	struct ArrayCount<T[], N> : ArrayCount<T, N - 1> { };

	template <typename T, uint32 I>
	struct ArrayCount<T[I], 0>
	{
		static const uint32 Value = I;
	};

	template <typename T, uint32 I, uint32 N>
	struct ArrayCount<T[I], N> : ArrayCount<T, N - 1> { };

}




