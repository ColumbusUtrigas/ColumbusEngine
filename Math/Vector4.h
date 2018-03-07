/************************************************
*                   Vector4.h                   *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#pragma once

#include <cmath>
#include <glm/glm.hpp>
#include <System/Random.h>
#include <Math/MathUtil.h>

namespace Columbus
{

	class C_Vector4;
	typedef C_Vector4 vec4;

	class C_Vector4
	{
	public:
		float x = 0;
		float y = 0;
		float z = 0;
		float w = 0;
		////////////////////////////////////////////////////////////////////////////
		inline explicit C_Vector4() :
			x(0),
			y(0),
			z(0),
			w(0)
		{}
		////////////////////////////////////////////////////////////////////////////
		inline explicit C_Vector4(const float aX, const float aY, const float aZ, const float aW) :
			x(static_cast<float>(aX)),
			y(static_cast<float>(aY)),
			z(static_cast<float>(aZ)),
			w(static_cast<float>(aW))
		{}
		////////////////////////////////////////////////////////////////////////////
		inline explicit C_Vector4(const glm::vec4 aVec) :
			x(static_cast<float>(aVec.x)),
			y(static_cast<float>(aVec.y)),
			z(static_cast<float>(aVec.z)),
			w(static_cast<float>(aVec.w))
		{}
		////////////////////////////////////////////////////////////////////////////
		//Conversion from GLM vector
		inline void fromGLM(const glm::vec4 aVec)
		{
			x = static_cast<float>(aVec.x);
			y = static_cast<float>(aVec.y);
			z = static_cast<float>(aVec.z);
			w = static_cast<float>(aVec.w);
		}
		////////////////////////////////////////////////////////////////////////////
		//Conversion to GLM vector
		inline glm::vec4 toGLM()
		{
			return glm::vec4(x, y, z, w);
		}
		////////////////////////////////////////////////////////////////////////////
		//Opeator =
		inline C_Vector4& operator=(C_Vector4 aOther)
		{
			x = aOther.x;
			y = aOther.y;
			z = aOther.z;
			w = aOther.w;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Opeator +
		inline C_Vector4 operator+(const C_Vector4 aOther)
		{
			return C_Vector4(x + aOther.x, y + aOther.y, z + aOther.z, w + aOther.w);
		}
		////////////////////////////////////////////////////////////////////////////
		//Opeator -
		inline C_Vector4 operator-(const C_Vector4 aOther)
		{
			return C_Vector4(x - aOther.x, y - aOther.y, z - aOther.z, w - aOther.w);
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator -
		inline C_Vector4 operator-()
		{
			return C_Vector4(-x, -y, -z, -w);
		}
		////////////////////////////////////////////////////////////////////////////
		//Opeator *
		inline C_Vector4 operator*(const C_Vector4 aOther)
		{
			return C_Vector4(x * aOther.x, y * aOther.y, z * aOther.z, w * aOther.w);
		}
		////////////////////////////////////////////////////////////////////////////
		//Opeator *
		inline C_Vector4 operator*(const float aOther)
		{
			return C_Vector4(x * aOther, y * aOther, z * aOther, w * aOther);
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator *
		inline friend C_Vector4 operator*(float aL, const C_Vector4 aR)
		{
			return C_Vector4(aL * aR.x, aL * aR.y, aL * aR.z, aL * aR.w);
		}
		////////////////////////////////////////////////////////////////////////////
		//Opeator /
		inline C_Vector4 operator/(const C_Vector4 aOther)
		{
			return C_Vector4(x / aOther.x, y / aOther.y, z / aOther.z, w / aOther.w);
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator /
		inline C_Vector4 operator/(const float aOther)
		{
			return C_Vector4(x / aOther, y / aOther, z / aOther, w / aOther);
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator ==
		inline bool operator==(const C_Vector4 aOther)
		{
			return (x == aOther.x && y == aOther.y && z == aOther.z && w == aOther.w);
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator !=
		inline bool operator!=(const C_Vector4 aOther)
		{
			return (x != aOther.x && y != aOther.y && z != aOther.z && w != aOther.w);
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator +=
		inline C_Vector4 operator+=(const C_Vector4 aOther)
		{
			x += aOther.x;
			y += aOther.y;
			z += aOther.z;
			w += aOther.w;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator -=
		inline C_Vector4 operator-=(const C_Vector4 aOther)
		{
			x -= aOther.x;
			y -= aOther.y;
			z -= aOther.z;
			w -= aOther.w;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator *=
		inline C_Vector4 operator*=(const C_Vector4 aOther)
		{
			x *= aOther.x;
			y *= aOther.y;
			z *= aOther.z;
			w *= aOther.w;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator *=
		inline C_Vector4 operator*=(const float aOther)
		{
			x *= aOther;
			y *= aOther;
			z *= aOther;
			w *= aOther;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator /=
		inline C_Vector4 operator/=(const C_Vector4 aOther)
		{
			x /= aOther.x;
			y /= aOther.y;
			z /= aOther.z;
			w /= aOther.w;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator /=
		inline C_Vector4 operator/=(const float aOther)
		{
			const float Scalar = 1.0f / aOther;
			x *= Scalar;
			y *= Scalar;
			z *= Scalar;
			w *= Scalar;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Return random from two vectors
		inline static C_Vector4 random(const C_Vector4 aMin, const C_Vector4 aMax)
		{
			C_Vector4 ret;
			ret.x = C_Random::range(aMin.x, aMax.x);
			ret.y = C_Random::range(aMin.y, aMax.y);
			ret.z = C_Random::range(aMin.z, aMax.z);
			ret.w = C_Random::range(aMin.w, aMax.w);
			return ret;
		}
		////////////////////////////////////////////////////////////////////////////
		//Return length of vector
		inline float length()
		{
			return sqrt((x * x) + (y * y) + (z * z) + (w * w));
		}
		////////////////////////////////////////////////////////////////////////////
		//Return length between two vectors
		inline float length(const C_Vector4 aVec)
		{
			return sqrt(pow(aVec.x - x, 2) + pow(aVec.y - y, 2) + pow(aVec.z - z, 2) + pow(aVec.w - w, 2));
		}
		////////////////////////////////////////////////////////////////////////////
		//Return normalize vector
		inline C_Vector4 normalize()
		{
			float l = sqrt((x * x) + (y * y) + (z * z) + (w * w));
			return C_Vector4(x / l, y / l, z / l, w / l);
		}
		////////////////////////////////////////////////////////////////////////////
		//Return dot product of 2 vectors
		inline float dot(C_Vector4 aOther)
		{
			return x * aOther.x + y * aOther.y + z * aOther.z + w * aOther.w;
		}
		////////////////////////////////////////////////////////////////////////////
		//Return dot product of 2 vectors
		inline static float dot(C_Vector4 aV1, C_Vector4 aV2)
		{
			return aV1.x * aV2.x + aV1.y * aV2.y + aV1.z * aV2.z + aV1.w * aV2.w;
		}
		////////////////////////////////////////////////////////////////////////////
		inline static C_Vector4 clamp(C_Vector4 a, C_Vector4 aMin, C_Vector4 aMax)
		{
			a.x = Clamp(a.x, aMin.x, aMax.x);
			a.y = Clamp(a.y, aMin.y, aMax.y);
			a.z = Clamp(a.z, aMin.z, aMax.z);
			a.w = Clamp(a.w, aMin.w, aMax.w);
			return a;
		}
		////////////////////////////////////////////////////////////////////////////
		inline ~C_Vector4() {}
	};

}





