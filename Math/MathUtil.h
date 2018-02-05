/************************************************
*                   MathUtil.h                  *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   04.02.2018                  *
*************************************************/
#pragma once

namespace Columbus
{

	template <class T>
	T Min(const T a, const T b)
	{
		return a < b ? a : b;
	}

	template <class T>
	T Max(const T a, const T b)
	{
		return a > b ? a : b;
	}

	template <class T>
	T Clamp(const T a, const T minValue, const T maxValue)
	{
		return Min(Max(a, minValue), maxValue);
	}

	template <class T>
	T Radians(const T degrees)
	{
		return degrees * 3.141592659 / 180.0f;
	}

	template <class T>
	T Degrees(const T radians)
	{
		return T * 3.141592659 / 180.0f;
	}

}



