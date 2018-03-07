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

#include <cstdlib>
#include <cmath>

namespace Columbus
{

	inline int32_t TruncToInt(const float Value) { return (int32_t)Value; }
	inline float TruncToFloat(const double Value) { return (float)Value; }
	inline int32_t FloorToInt(const float Value) { return (int32_t)floorf(Value); }
	inline float FloorToFloat(const float Value) { return floorf(Value); }
	inline double FloorToDouble(const double Value) { return floor(Value); }
	inline int32_t RoundToInt(const float Value) { FloorToInt(Value + 0.5f); }
	inline float RoundToFloat(const float Value) { FloorToFloat(Value + 0.5f); }
	inline double RoundToDouble(const double Value) { FloorToDouble(Value + 0.5); }
	inline int32_t CeilToInt(const float Value) { return TruncToInt(ceilf(Value)); }
	inline float CeilToFloat(const float Value) { return ceilf(Value); }
	inline double CeilToDouble(const double Value) { return ceil(Value); }

	//e ^ Value
	inline float Exp(const float Value) { return expf(Value); }
	//2 ^ value
	inline float Exp2(const float Value) { return powf(2.0f, Value); }

	inline float Sin(const float Value)	{ return sinf(Value); }
	inline float Cos(const float Value) { return cosf(Value); }
	inline float Tan(const float Value) { return tanf(Value); }
	inline float Sqrt(const float Value) { return sqrtf(Value); }
	inline float Pow(const float A, const float B) { return powf(A, B); };

	inline bool IsPowerOf2(const int32_t Value) { return (Value & (Value - 1)) == 0; }

	template <class T>
	inline T Abs(const T Value)
	{
		return (Value >= (T)0) ? Value : -Value;
	}

	template <class T>
	inline T Min(const T a, const T b)
	{
		return a < b ? a : b;
	}

	template <class T>
	inline T Max(const T a, const T b)
	{
		return a > b ? a : b;
	}
	//Constrain a value to lie between two further values
	template <class T>
	inline T Clamp(const T a, const T MinValue, const T MaxValue)
	{
		return Min(Max(a, MinValue), MaxValue);
	}

	//Converts degrees to radians
	template <class T>
	inline T Radians(const T Degrees)
	{
		return Degrees * 3.141592653f / 180.0f;
		//0.017453293 = 3.141592653 / 180.0
		//return Degrees * 0.017453293f;
	}

	//Converts radians to degrees
	template <class T>
	inline T Degrees(const T Radians)
	{
		//return Radians * 180.0f / 3.141592653f;
		//57.295779524 = 180.0 / 3.141592653
		return Radians * 57.295779524f;
	}

}



