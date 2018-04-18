#pragma once

namespace Columbus
{

	template <typename T>
	struct IsEnum
	{
		static const bool Value = __is_enum(T);
	};

}











