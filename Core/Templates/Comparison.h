#pragma once

#include <Core/Platform/Platform.h>

namespace Columbus
{

	/*
	* Comparison predicates
	*/

	/*
	* @see: http://en.cppreference.com/w/cpp/utility/functional/greater
	*/
	template <typename T>
	struct Greater
	{
		constexpr inline bool operator()(const T& A, const T& B) const
		{
			return A > B;
		}
	};
	/*
	* @see: http://en.cppreference.com/w/cpp/utility/functional/less
	*/
	template <typename T>
	struct Less
	{
		constexpr inline bool operator()(const T& A, const T& B) const
		{
			return A < B;
		}
	};
	/*
	* @see: http://en.cppreference.com/w/cpp/algorithm/equal
	*/
	template <typename InputIterator1, typename InputIterator2>
	constexpr inline bool Equal(InputIterator1 First1, InputIterator1 Last1, InputIterator2 First2)
	{
		while (First1 != Last1)
		{
			if (!(*First1 == *First2))
			{
				return false;
			}

			++First1;
			++First2;
		}

		return true;
	}


}












