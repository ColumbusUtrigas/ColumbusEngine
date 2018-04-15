#pragma once

namespace Columbus
{

	template<typename A, typename B>
	struct AreTypesEqual;

	template<typename, typename>
	struct AreTypesEqual
	{
		enum { Value = false };
	};

	template<typename A>
	struct AreTypesEqual<A, A>
	{
		enum { Value = true };
	};


	#define ARE_TYPES_EQUAL(A, B) AreTypesEqual<A, B>::Value
	
}











