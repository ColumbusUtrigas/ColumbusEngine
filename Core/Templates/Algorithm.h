#pragma once

#include <algorithm>

namespace Columbus
{

	template <typename InputIterator, typename OutputIterator>
	constexpr OutputIterator Move(InputIterator InFirst, InputIterator InLast, OutputIterator OutFirst)
	{
		while (InFirst != InLast)
		{
			*OutFirst++ = std::move(*InFirst++);
		}

		return OutFirst;
	}

}










