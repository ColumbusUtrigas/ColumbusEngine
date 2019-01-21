#pragma once

namespace Columbus
{

	template <typename Iterator, typename Predicate>
	Iterator LinearSearch(Iterator Begin, Iterator End, Predicate Pred)
	{
		while (Begin != End)
		{
			if (Pred(*Begin))
			{
				return Begin;
			}

			Begin++;
		}

		return End;
	}

}


