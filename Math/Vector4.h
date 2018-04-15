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

	class Vector4;
	typedef Vector4 vec4;

	class Vector4
	{
	public:
		float x = 0;
		float y = 0;
		float z = 0;
		float w = 0;
		////////////////////////////////////////////////////////////////////////////
		FORCEINLINE explicit Vector4() :
			x(0),
			y(0),
			z(0),
			w(0)
		{}
		////////////////////////////////////////////////////////////////////////////
		FORCEINLINE explicit Vector4(const float aX, const float aY, const float aZ, const float aW) :
			x(static_cast<float>(aX)),
			y(static_cast<float>(aY)),
			z(static_cast<float>(aZ)),
			w(static_cast<float>(aW))
		{}
		////////////////////////////////////////////////////////////////////////////
		FORCEINLINE explicit Vector4(const glm::vec4 aVec) :
			x(static_cast<float>(aVec.x)),
			y(static_cast<float>(aVec.y)),
			z(static_cast<float>(aVec.z)),
			w(static_cast<float>(aVec.w))
		{}
		////////////////////////////////////////////////////////////////////////////
		//Conversion from GLM vector
		FORCEINLINE void fromGLM(const glm::vec4 aVec)
		{
			x = static_cast<float>(aVec.x);
			y = static_cast<float>(aVec.y);
			z = static_cast<float>(aVec.z);
			w = static_cast<float>(aVec.w);
		}
		////////////////////////////////////////////////////////////////////////////
		//Conversion to GLM vector
		FORCEINLINE glm::vec4 toGLM()
		{
			return glm::vec4(x, y, z, w);
		}
		////////////////////////////////////////////////////////////////////////////
		//Opeator =
		FORCEINLINE Vector4& operator=(Vector4 aOther)
		{
			x = aOther.x;
			y = aOther.y;
			z = aOther.z;
			w = aOther.w;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Opeator +
		FORCEINLINE Vector4 operator+(const Vector4 aOther)
		{
			return Vector4(x + aOther.x, y + aOther.y, z + aOther.z, w + aOther.w);
		}
		////////////////////////////////////////////////////////////////////////////
		//Opeator -
		FORCEINLINE Vector4 operator-(const Vector4 aOther)
		{
			return Vector4(x - aOther.x, y - aOther.y, z - aOther.z, w - aOther.w);
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator -
		FORCEINLINE Vector4 operator-()
		{
			return Vector4(-x, -y, -z, -w);
		}
		////////////////////////////////////////////////////////////////////////////
		//Opeator *
		FORCEINLINE Vector4 operator*(const Vector4 aOther)
		{
			return Vector4(x * aOther.x, y * aOther.y, z * aOther.z, w * aOther.w);
		}
		////////////////////////////////////////////////////////////////////////////
		//Opeator *
		FORCEINLINE Vector4 operator*(const float aOther)
		{
			return Vector4(x * aOther, y * aOther, z * aOther, w * aOther);
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator *
		FORCEINLINE friend Vector4 operator*(float aL, const Vector4 aR)
		{
			return Vector4(aL * aR.x, aL * aR.y, aL * aR.z, aL * aR.w);
		}
		////////////////////////////////////////////////////////////////////////////
		//Opeator /
		FORCEINLINE Vector4 operator/(const Vector4 aOther)
		{
			return Vector4(x / aOther.x, y / aOther.y, z / aOther.z, w / aOther.w);
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator /
		FORCEINLINE Vector4 operator/(const float aOther)
		{
			return Vector4(x / aOther, y / aOther, z / aOther, w / aOther);
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator ==
		FORCEINLINE bool operator==(const Vector4 aOther)
		{
			return (x == aOther.x && y == aOther.y && z == aOther.z && w == aOther.w);
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator !=
		FORCEINLINE bool operator!=(const Vector4 aOther)
		{
			return (x != aOther.x && y != aOther.y && z != aOther.z && w != aOther.w);
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator +=
		FORCEINLINE Vector4 operator+=(const Vector4 aOther)
		{
			x += aOther.x;
			y += aOther.y;
			z += aOther.z;
			w += aOther.w;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator -=
		FORCEINLINE Vector4 operator-=(const Vector4 aOther)
		{
			x -= aOther.x;
			y -= aOther.y;
			z -= aOther.z;
			w -= aOther.w;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator *=
		FORCEINLINE Vector4 operator*=(const Vector4 aOther)
		{
			x *= aOther.x;
			y *= aOther.y;
			z *= aOther.z;
			w *= aOther.w;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator *=
		FORCEINLINE Vector4 operator*=(const float aOther)
		{
			x *= aOther;
			y *= aOther;
			z *= aOther;
			w *= aOther;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator /=
		FORCEINLINE Vector4 operator/=(const Vector4 aOther)
		{
			x /= aOther.x;
			y /= aOther.y;
			z /= aOther.z;
			w /= aOther.w;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator /=
		FORCEINLINE Vector4 operator/=(const float aOther)
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
		FORCEINLINE static Vector4 random(const Vector4 aMin, const Vector4 aMax)
		{
			Vector4 ret;
			ret.x = Random::range(aMin.x, aMax.x);
			ret.y = Random::range(aMin.y, aMax.y);
			ret.z = Random::range(aMin.z, aMax.z);
			ret.w = Random::range(aMin.w, aMax.w);
			return ret;
		}
		////////////////////////////////////////////////////////////////////////////
		//Return length of vector
		FORCEINLINE float length()
		{
			return sqrt((x * x) + (y * y) + (z * z) + (w * w));
		}
		////////////////////////////////////////////////////////////////////////////
		//Return length between two vectors
		FORCEINLINE float length(const Vector4 aVec)
		{
			return sqrt(pow(aVec.x - x, 2) + pow(aVec.y - y, 2) + pow(aVec.z - z, 2) + pow(aVec.w - w, 2));
		}
		////////////////////////////////////////////////////////////////////////////
		//Return normalize vector
		FORCEINLINE Vector4 normalize()
		{
			float l = sqrt((x * x) + (y * y) + (z * z) + (w * w));
			return Vector4(x / l, y / l, z / l, w / l);
		}
		////////////////////////////////////////////////////////////////////////////
		//Return dot product of 2 vectors
		FORCEINLINE float dot(Vector4 aOther)
		{
			return x * aOther.x + y * aOther.y + z * aOther.z + w * aOther.w;
		}
		////////////////////////////////////////////////////////////////////////////
		//Return dot product of 2 vectors
		FORCEINLINE static float dot(Vector4 aV1, Vector4 aV2)
		{
			return aV1.x * aV2.x + aV1.y * aV2.y + aV1.z * aV2.z + aV1.w * aV2.w;
		}
		////////////////////////////////////////////////////////////////////////////
		FORCEINLINE static Vector4 clamp(Vector4 a, Vector4 aMin, Vector4 aMax)
		{
			a.x = Math::Clamp(a.x, aMin.x, aMax.x);
			a.y = Math::Clamp(a.y, aMin.y, aMax.y);
			a.z = Math::Clamp(a.z, aMin.z, aMax.z);
			a.w = Math::Clamp(a.w, aMin.w, aMax.w);
			return a;
		}
		////////////////////////////////////////////////////////////////////////////
		~Vector4() { }
	};

}





