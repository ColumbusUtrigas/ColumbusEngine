#pragma once

namespace Columbus
{

	template <typename T>
	struct TIsAbstract
	{
		enum { Value = __is_abstract(T) };
	};
	
}








