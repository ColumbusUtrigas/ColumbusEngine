#pragma once

#include <Core/Templates/EnableIf.h>
#include <Core/Templates/HasTrivialDestructor.h>
#include <Core/Types.h>

namespace Columbus
{

	/*
	* Functions destruct elements if they have non-trivial destructor
	*/

	/*
	* Destruct single element
	* @param Type* Element: Pointer to element
	*/
	template <typename Type>
	typename EnableIf<!HasTrivialDestructor<Type>::Value>::Type DestructElement(Type* Element)
	{
		//VC++ doesn't compiling without this
		typedef Type Destructor;
		Element->Destructor::~Destructor();
	}

	template <typename Type>
	typename EnableIf<HasTrivialDestructor<Type>::Value>::Type DestructElement(Type* Element)
	{

	}
	/*
	* Destruct range of elements
	* @param Type* Elements: Array of elements, which should be destructed
	* @param uint32 Count: Count of elements in array
	*/
	template <typename Type>
	typename EnableIf<!HasTrivialDestructor<Type>::Value>::Type DestructElements(Type* Elements, uint32 Count)
	{
		typedef Type Destructor;

		while (Count)
		{
			Elements->Destructor::~Destructor();
			++Elements;
			--Count;
		}
	}

	template <typename Type>
	typename EnableIf<HasTrivialDestructor<Type>::Value>::Type DestructElements(Type* Elements, uint32 Count)
	{
		
	}

}










