#pragma once

#include <stdlib.h>
#include <string.h>
#include <string>
#include <algorithm>

namespace Columbus
{

	using String = std::string;

	static std::string str_tolower(const std::string& str)
	{
		auto res = str;
		std::transform(res.begin(), res.end(), res.begin(), ::tolower);
		return res;
	}

}
