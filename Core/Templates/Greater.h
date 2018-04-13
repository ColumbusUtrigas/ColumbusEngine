#pragma once

#include <Core/Platform/Platform.h>

namespace Columbus
{

	template <typename T = void>
	struct TGreater
	{
		FORCEINLINE bool operator()(const T& A, const T& B) const
		{
			return B < A;
		}
	};

	template <>
	struct TGreater<void>
	{
		template <typename T>
		FORCEINLINE bool operator()(const T& A, const T& B) const
		{
			return B < A;
		}
	};

}












