#pragma once

namespace Columbus
{

	template <bool Predicate, typename Result = void>
	class TEnableIf;

	template<typename Result>
	struct TEnableIf<true, Result>
	{
		typedef Result Type;
	};

	template<typename Result>
	struct TEnableIf<false, Result>
	{ };

}











