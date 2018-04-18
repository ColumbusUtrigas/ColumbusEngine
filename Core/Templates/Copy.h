#pragma once

namespace Columbus
{
	/*
	* Analog of std::copy
	* @see: http://en.cppreference.com/w/cpp/algorithm/copy
	*/
	template <typename InputIterator, typename OutputIterator>
	OutputIterator Copy(InputIterator InFirst, InputIterator InLast, OutputIterator OutFirst)
	{
		while (InFirst != InLast)
		{
			*OutFirst++ = *InFirst++;
		}

		return OutFirst;
	}
	/*
	* Analog of std:copy_if
	* @see: http://en.cppreference.com/w/cpp/algorithm/copy
	*/
	template <typename InputIterator, typename OutputIterator, typename Predicate>
	OutputIterator Copy(InputIterator InFirst, InputIterator InLast, OutputIterator OutFirst, Predicate InPredicate)
	{
		while (InFirst != InLast)
		{
			if (InPredicate(*InFirst))
			{
				*OutFirst++ = *InFirst++;
			}
		}

		return OutFirst;
	}

}










