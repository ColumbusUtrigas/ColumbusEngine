#pragma once

namespace Columbus
{

	template <bool Predicate, typename Result = void>
	class EnableIf;

	template<typename Result>
	struct EnableIf<true, Result>
	{
		typedef Result Type;
	};

	template<typename Result>
	struct EnableIf<false, Result>
	{ };

}











