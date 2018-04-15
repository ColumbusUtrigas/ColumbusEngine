#pragma once

#include <Core/Types.h>

namespace Columbus
{

	template <typename T>
	struct IsArithmetic
	{ 
		enum { Value = false };
	};

	template <> struct IsArithmetic<float>       { enum { Value = true }; };
	template <> struct IsArithmetic<double>      { enum { Value = true }; };
	template <> struct IsArithmetic<long double> { enum { Value = true }; };
	template <> struct IsArithmetic<uint8>       { enum { Value = true }; };
	template <> struct IsArithmetic<uint16>      { enum { Value = true }; };
	template <> struct IsArithmetic<uint32>      { enum { Value = true }; };
	template <> struct IsArithmetic<uint64>      { enum { Value = true }; };
	template <> struct IsArithmetic<int8>        { enum { Value = true }; };
	template <> struct IsArithmetic<int16>       { enum { Value = true }; };
	template <> struct IsArithmetic<int32>       { enum { Value = true }; };
	template <> struct IsArithmetic<int64>       { enum { Value = true }; };
	template <> struct IsArithmetic<bool>        { enum { Value = true }; };

	template <typename T> struct IsArithmetic<const          T> { enum { Value = IsArithmetic<T>::Value }; };
	template <typename T> struct IsArithmetic<      volatile T> { enum { Value = IsArithmetic<T>::Value }; };
	template <typename T> struct IsArithmetic<const volatile T> { enum { Value = IsArithmetic<T>::Value }; };

}









