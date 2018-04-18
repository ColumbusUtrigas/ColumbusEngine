#pragma once

#include <Core/Types.h>

namespace Columbus
{

	template <typename T>
	struct IsArithmetic
	{ 
		static const bool Value = false;
	};

	template <> struct IsArithmetic<float>       { static const bool Value = true; };
	template <> struct IsArithmetic<double>      { static const bool Value = true; };
	template <> struct IsArithmetic<long double> { static const bool Value = true; };
	template <> struct IsArithmetic<uint8>       { static const bool Value = true; };
	template <> struct IsArithmetic<uint16>      { static const bool Value = true; };
	template <> struct IsArithmetic<uint32>      { static const bool Value = true; };
	template <> struct IsArithmetic<uint64>      { static const bool Value = true; };
	template <> struct IsArithmetic<int8>        { static const bool Value = true; };
	template <> struct IsArithmetic<int16>       { static const bool Value = true; };
	template <> struct IsArithmetic<int32>       { static const bool Value = true; };
	template <> struct IsArithmetic<int64>       { static const bool Value = true; };
	template <> struct IsArithmetic<bool>        { static const bool Value = true; };

	template <typename T> struct IsArithmetic<const          T> { static const bool Value = IsArithmetic<T>::Value; };
	template <typename T> struct IsArithmetic<      volatile T> { static const bool Value = IsArithmetic<T>::Value; };
	template <typename T> struct IsArithmetic<const volatile T> { static const bool Value = IsArithmetic<T>::Value; };

}









