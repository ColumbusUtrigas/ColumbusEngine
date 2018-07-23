#pragma once

namespace Columbus
{

	template <typename T>
	struct IsAbstract
	{
		static const bool Value = __is_abstract(T);
	};
	
}








