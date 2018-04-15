#pragma once

#include <Core/Platform/Platform.h>

namespace Columbus
{

	template <typename T = void>
	struct Greater
	{
		inline bool operator()(const T& A, const T& B) const
		{
			return B < A;
		}
	};

	template <>
	struct Greater<void>
	{
		template <typename T>
		inline bool operator()(const T& A, const T& B) const
		{
			return B < A;
		}
	};

}












