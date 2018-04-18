#pragma once

namespace Columbus
{

	template<typename A, typename B>
	struct AreTypesEqual;

	template<typename, typename>
	struct AreTypesEqual
	{
		static const bool Value = false;
	};

	template<typename A>
	struct AreTypesEqual<A, A>
	{
		static const bool Value = true;
	};


	#define ARE_TYPES_EQUAL(A, B) AreTypesEqual<A, B>::Value
	
}











