/************************************************
*                   Vector3.h                   *
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

	class Vector3;
	typedef Vector3 vec3;

	class Vector3
	{
	public:
		float x = 0;
		float y = 0;
		float z = 0;
		////////////////////////////////////////////////////////////////////////////
		inline explicit Vector3() :
			x(0),
			y(0),
			z(0)
		{}
		////////////////////////////////////////////////////////////////////////////
		inline explicit Vector3(const float aX, const float aY, const float aZ) :
			x(static_cast<float>(aX)),
			y(static_cast<float>(aY)),
			z(static_cast<float>(aZ))
		{}
		////////////////////////////////////////////////////////////////////////////
		inline explicit Vector3(const glm::vec3 aVec) :
			x(static_cast<float>(aVec.x)),
			y(static_cast<float>(aVec.y)),
			z(static_cast<float>(aVec.z))
		{}
		////////////////////////////////////////////////////////////////////////////
		//Conversion from GLM vector
		inline void fromGLM(const glm::vec3 aVec)
		{
			x = static_cast<float>(aVec.x);
			y = static_cast<float>(aVec.y);
			z = static_cast<float>(aVec.z);
		}
		////////////////////////////////////////////////////////////////////////////
		//Conversion to GLM vector
		inline glm::vec3 toGLM()
		{
			return glm::vec3(x, y, z);
		}
		////////////////////////////////////////////////////////////////////////////
		//Opeator =
		inline Vector3& operator=(Vector3 aOther)
		{
			x = aOther.x;
			y = aOther.y;
			z = aOther.z;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Opeator +
		inline Vector3 operator+(const Vector3 aOther) const
		{
			return Vector3(x + aOther.x, y + aOther.y, z + aOther.z);
		}
		////////////////////////////////////////////////////////////////////////////
		//Opeator -
		inline Vector3 operator-(const Vector3 aOther) const
		{
			return Vector3(x - aOther.x, y - aOther.y, z - aOther.z);
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator -
		inline Vector3 operator-() const
		{
			return Vector3(-x, -y, -z);
		}
		////////////////////////////////////////////////////////////////////////////
		//Opeator *
		inline Vector3 operator*(const Vector3 aOther) const
		{
			return Vector3(x * aOther.x, y * aOther.y, z * aOther.z);
		}
		////////////////////////////////////////////////////////////////////////////
		//Opeator *
		inline Vector3 operator*(const float aOther) const
		{
			return Vector3(x * aOther, y * aOther, z * aOther);
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator *
		inline friend Vector3 operator*(float aL, const Vector3 aR)
		{
			return Vector3(aL * aR.x, aL * aR.y, aL * aR.z);
		}
		////////////////////////////////////////////////////////////////////////////
		//Opeator /
		inline Vector3 operator/(const Vector3 aOther) const
		{
			return Vector3(x / aOther.x, y / aOther.y, z / aOther.z);
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator /
		inline Vector3 operator/(const float aOther) const
		{
			return Vector3(x / aOther, y / aOther, z / aOther);
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator ==
		inline bool operator==(const Vector3 aOther) const
		{
			return (x == aOther.x && y == aOther.y && z == aOther.z);
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator !=
		inline bool operator!=(const Vector3 aOther) const
		{
			return (x != aOther.x && y != aOther.y && z != aOther.z);
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator +=
		inline Vector3 operator+=(const Vector3 aOther)
		{
			x += aOther.x;
			y += aOther.y;
			z += aOther.z;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator -=
		inline Vector3 operator-=(const Vector3 aOther)
		{
			x -= aOther.x;
			y -= aOther.y;
			z -= aOther.z;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator *=
		inline Vector3 operator*=(const Vector3 aOther)
		{
			x *= aOther.x;
			y *= aOther.y;
			z *= aOther.z;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator *=
		inline Vector3 operator*=(const float aOther)
		{
			x *= aOther;
			y *= aOther;
			z *= aOther;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator /=
		inline Vector3 operator/=(const Vector3 aOther)
		{
			x /= aOther.x;
			y /= aOther.y;
			z /= aOther.z;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator /=
		inline Vector3 operator/=(const float aOther)
		{
			const float Scalar = 1.0f / aOther;
			x *= Scalar;
			y *= Scalar;
			z *= Scalar;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Return random from two vectors
		inline static Vector3 random(const Vector3 aMin, const Vector3 aMax)
		{
			Vector3 ret;
			ret.x = Random::range(aMin.x, aMax.x);
			ret.y = Random::range(aMin.y, aMax.y);
			ret.z = Random::range(aMin.z, aMax.z);
			return ret;
		}
		////////////////////////////////////////////////////////////////////////////
		//Return length of vector
		inline float length()
		{
			return sqrt((x * x) + (y * y) + (z * z));
		}
		////////////////////////////////////////////////////////////////////////////
		//Return length between 2 vectors
		inline float length(const Vector3 aVec)
		{
			return sqrt(pow(aVec.x - x, 2) + pow(aVec.y - y, 2) + pow(aVec.z - z, 2));
		}
		////////////////////////////////////////////////////////////////////////////
		//Return normalize vector
		inline Vector3 normalize()
		{
			float l = sqrt((x * x) + (y * y) + (z * z));
			return Vector3(x / l, y / l, z / l);
		}
		////////////////////////////////////////////////////////////////////////////
		//Return dot product of 2 vectors
		inline float dot(Vector3 aOther)
		{
			return x * aOther.x + y * aOther.y + z * aOther.z;
		}
		////////////////////////////////////////////////////////////////////////////
		//Return dot product of 2 vectors
		inline static float dot(Vector3 aV1, Vector3 aV2)
		{
			return aV1.x * aV2.x + aV1.y * aV2.y + aV1.z * aV2.z;
		}
		////////////////////////////////////////////////////////////////////////////
		//Return cross product of 2 vectors
		inline Vector3 cross(Vector3 aOther)
		{
			return cross(*this, aOther);
		}
		////////////////////////////////////////////////////////////////////////////
		//Return cross product of 2 vectors
		inline static Vector3 cross(Vector3 aV1, Vector3 aV2)
		{
			Vector3 ret;
			ret.x = aV1.y * aV2.z - aV1.z * aV2.y;
			ret.y = aV1.z * aV2.x - aV1.x * aV2.z;
			ret.z = aV1.x * aV2.y - aV1.y * aV2.x;
			return ret;
		}
		////////////////////////////////////////////////////////////////////////////
		inline static Vector3 clamp(Vector3 a, Vector3 aMin, Vector3 aMax)
		{
			a.x = Clamp(a.x, aMin.x, aMax.x);
			a.y = Clamp(a.y, aMin.y, aMax.y);
			a.z = Clamp(a.z, aMin.z, aMax.z);
			return a;
		}
		////////////////////////////////////////////////////////////////////////////
		//Destructor
		inline ~Vector3() {}
	};

}









