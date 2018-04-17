#pragma once

namespace Columbus
{

	template <typename T, T Val>
	struct IntegralConstant
	{
		static const T Value = Val;

		constexpr operator T() noexcept { return Value; }
		constexpr T operator()() noexcept { return Value; }
	};

}





