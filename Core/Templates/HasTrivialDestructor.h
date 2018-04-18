#pragma once

namespace Columbus
{

	template <typename T>
	struct HasTrivialDestructor
	{
		static const bool Value = __has_trivial_destructor(T);
	};

}














