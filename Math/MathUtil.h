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

#include <Core/Types.h>

#include <cstdlib>
#include <cmath>

namespace Columbus
{

	inline int32 TruncToInt(const float Value) { return (int32)Value; }
	inline float TruncToFloat(const double Value) { return (float)Value; }
	inline int32 FloorToInt(const float Value) { return (int32)floorf(Value); }
	inline float FloorToFloat(const float Value) { return floorf(Value); }
	inline double FloorToDouble(const double Value) { return floor(Value); }
	inline int32 RoundToInt(const float Value) { return FloorToInt(Value + 0.5f); }
	inline float RoundToFloat(const float Value) { return FloorToFloat(Value + 0.5f); }
	inline double RoundToDouble(const double Value) { return FloorToDouble(Value + 0.5); }
	inline int32 CeilToInt(const float Value) { return TruncToInt(ceilf(Value)); }
	inline float CeilToFloat(const float Value) { return ceilf(Value); }
	inline double CeilToDouble(const double Value) { return ceil(Value); }

	/** e ^ Value */
	inline float Exp(const float Value) { return expf(Value); }
	/** 2 ^ value */
	inline float Exp2(const float Value) { return powf(2.0f, Value); }

	inline float Sin(const float Value)	{ return sinf(Value); }
	inline float Cos(const float Value) { return cosf(Value); }
	inline float Tan(const float Value) { return tanf(Value); }
	inline float Sqrt(const float Value) { return sqrtf(Value); }
	inline float Pow(const float A, const float B) { return powf(A, B); };

	/*
	* Find sinus and cosine of Value
	* @param const float Value: Value for finding sinus and cosine
	* @param float& S: Sinus value reference
	* @param float& C: Cosine value reference
	*/
	inline void SinCos(const float Value, float& S, float& C)
	{
		S = Sin(Value);
		C = Cos(Value);
	}
	/*
	* Return true if Value is power of 2
	*/
	inline bool IsPowerOf2(const int32 Value)
	{
		return (Value & (Value - 1)) == 0;
	}
	/*
	* Return true if Value is Nan (not a number)
	*/
	inline bool IsNan(float Value)
	{
		return ((*(uint32*)&Value) & 0x7FFFFFFF) > 0x7F800000;
	}
	/*
	* Return true if Value if finite (not NaN and not infinity)
	*/
	inline bool IsFinite(float Value)
	{
		return ((*(uint32*)&Value) & 0x7F800000) != 0x7F800000;
	}
	/*
	* Return absolute value in a generic way
	*/
	template <class T>
	inline T Abs(const T Value)
	{
		return (Value >= (T)0) ? Value : -Value;
	}
	/*
	* Returns 1, 0, or -1 depending on relation of T to 0
	*/
	template <class T>
	inline T Sign(const T A)
	{
		return (A > (T)0) ? (T)1 : ((A < (T)0) ? (T)-1 : (T)0);
	}
	/*
	* Return lower value in a generic way
	*/
	template <class T>
	inline T Min(const T A, const T B)
	{
		return A < B ? A : B;
	}
	/*
	* Return heigher value in a generic way
	*/
	template <class T>
	inline T Max(const T A, const T B)
	{
		return A > B ? A : B;
	}
	/*
	* Constrain a value to lie between two further values
	*/
	template <class T>
	inline T Clamp(const T A, const T MinValue, const T MaxValue)
	{
		return Min(Max(A, MinValue), MaxValue);
	}
	/*
	* Converts Degrees to radians
	*/
	template <class T>
	inline T Radians(const T Degrees)
	{
		//return Degrees * 3.141592653f / 180.0f;
		//0.017453293 = 3.141592653 / 180.0
		return Degrees * 0.017453293f;
	}

	/*
	* Converts Radians to degrees
	*/
	template <class T>
	inline T Degrees(const T Radians)
	{
		//return Radians * 180.0f / 3.141592653f;
		//57.295779524 = 180.0 / 3.141592653
		return Radians * 57.295779524f;
	}

}



