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
#include <Core/Platform/Platform.h>

#include <cstdlib>
#include <cmath>

namespace Columbus
{

	struct Math
	{
		static FORCEINLINE int32 TruncToInt(const float Value) { return (int32)Value; }
		static FORCEINLINE float TruncToFloat(const double Value) { return (float)Value; }
		static FORCEINLINE int32 FloorToInt(const float Value) { return (int32)floorf(Value); }
		static FORCEINLINE float FloorToFloat(const float Value) { return floorf(Value); }
		static FORCEINLINE double FloorToDouble(const double Value) { return floor(Value); }
		static FORCEINLINE int32 RoundToInt(const float Value) { return FloorToInt(Value + 0.5f); }
		static FORCEINLINE float RoundToFloat(const float Value) { return FloorToFloat(Value + 0.5f); }
		static FORCEINLINE double RoundToDouble(const double Value) { return FloorToDouble(Value + 0.5); }
		static FORCEINLINE int32 CeilToInt(const float Value) { return TruncToInt(ceilf(Value)); }
		static FORCEINLINE float CeilToFloat(const float Value) { return ceilf(Value); }
		static FORCEINLINE double CeilToDouble(const double Value) { return ceil(Value); }

		/** e ^ Value */
		static FORCEINLINE float Exp(const float Value) { return expf(Value); }
		/** 2 ^ value */
		static FORCEINLINE float Exp2(const float Value) { return powf(2.0f, Value); }

		static FORCEINLINE float Sin(const float Value) { return sinf(Value); }
		static FORCEINLINE float Cos(const float Value) { return cosf(Value); }
		static FORCEINLINE float Tan(const float Value) { return tanf(Value); }
		static FORCEINLINE float Sqrt(const float Value) { return sqrtf(Value); }
		static FORCEINLINE float Pow(const float A, const float B) { return powf(A, B); };

		/*
		* Find sinus and cosine of Value
		* @param const float Value: Value for finding sinus and cosine
		* @param float& S: Sinus value reference
		* @param float& C: Cosine value reference
		*/
		static FORCEINLINE void SinCos(const float Value, float& S, float& C)
		{
			S = Sin(Value);
			C = Cos(Value);
		}
		/*
		* Return true if Value is power of 2
		*/
		static FORCEINLINE bool IsPowerOf2(const int32 Value)
		{
			return (Value & (Value - 1)) == 0;
		}
		/*
		* Finding upper power of 2
		*/
		static FORCEINLINE int64 UpperPowerOf2(int64 Value)
		{
			Value--;
			Value |= Value >> 1;
			Value |= Value >> 2;
			Value |= Value >> 4;
			Value |= Value >> 8;
			Value |= Value >> 16;
			Value++;

			return Value;
		}
		/*
		* Return true if Value is Nan (not a number)
		*/
		static FORCEINLINE bool IsNan(float Value)
		{
			return ((*(uint32*)&Value) & 0x7FFFFFFF) > 0x7F800000;
		}
		/*
		* Return true if Value if finite (not NaN and not infinity)
		*/
		static FORCEINLINE bool IsFinite(float Value)
		{
			return ((*(uint32*)&Value) & 0x7F800000) != 0x7F800000;
		}
		/*
		* Return absolute value in a generic way
		*/
		template <class T>
		static FORCEINLINE T Abs(const T Value)
		{
			return (Value >= (T)0) ? Value : -Value;
		}
		/*
		* Returns 1, 0, or -1 depending on relation of T to 0
		*/
		template <class T>
		static FORCEINLINE T Sign(const T A)
		{
			return (A > (T)0) ? (T)1 : ((A < (T)0) ? (T)-1 : (T)0);
		}
		/*
		* Return lower value in a generic way
		*/
		template <class T>
		static FORCEINLINE T Min(const T A, const T B)
		{
			return A < B ? A : B;
		}
		/*
		* Return heigher value in a generic way
		*/
		template <class T>
		static FORCEINLINE T Max(const T A, const T B)
		{
			return A > B ? A : B;
		}
		/*
		* Constrain a value to lie between two further values
		*/
		template <class T>
		static FORCEINLINE T Clamp(const T A, const T MinValue, const T MaxValue)
		{
			return Min(Max(A, MinValue), MaxValue);
		}
		/*
		* Converts Degrees to radians
		*/
		template <class T>
		static FORCEINLINE T Radians(const T Degrees)
		{
			//return Degrees * 3.141592653f / 180.0f;
			//0.017453293 = 3.141592653 / 180.0
			return Degrees * 0.017453293f;
		}

		/*
		* Converts Radians to degrees
		*/
		template <class T>
		static FORCEINLINE T Degrees(const T Radians)
		{
			//return Radians * 180.0f / 3.141592653f;
			//57.295779524 = 180.0 / 3.141592653
			return Radians * 57.295779524f;
		}
	};

}



