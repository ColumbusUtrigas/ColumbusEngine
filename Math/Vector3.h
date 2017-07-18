#pragma once

#include <cmath>
#include <glm/glm.hpp>

namespace C
{

	class C_Vector3
	{
	public:
		float x = 0;
		float y = 0;
		float z = 0;

		inline C_Vector3() {}
		inline C_Vector3(float aX, float aY, float aZ) : x(aX), y(aY), z(aZ) {}
		inline C_Vector3(const glm::vec3 aVec) : x(aVec.x), y(aVec.y), z(aVec.z) {}

		inline void fromGLM(const glm::vec3 aVec)
		{
			x = aVec.x;
			y = aVec.y;
			z = aVec.z;
		}

		inline glm::vec3 toGLM()
		{
			return glm::vec3(x, y, z);
		}

		inline C_Vector3& operator=(C_Vector3 aOther)
		{
			x = aOther.x;
			y = aOther.y;
			z = aOther.z;
			return *this;
		}

		inline C_Vector3 operator+(const C_Vector3 aOther)
		{
			x += aOther.x;
			y += aOther.y;
			z += aOther.z;
			return *this;
		}

		inline C_Vector3 operator-(const C_Vector3 aOther)
		{
			x -= aOther.x;
			y -= aOther.y;
			z -= aOther.z;
			return *this;
		}

		inline C_Vector3 operator*(const C_Vector3 aOther)
		{
			x *= aOther.x;
			y *= aOther.y;
			z *= aOther.z;
			return *this;
		}

		inline C_Vector3 operator*(const float aOther)
		{
			x *= aOther;
			y *= aOther;
			z *= aOther;
			return *this;
		}

		inline C_Vector3 operator/(const C_Vector3 aOther)
		{
			x /= aOther.x;
			y /= aOther.y;
			z /= aOther.z;
			return *this;
		}

		inline bool operator==(C_Vector3 aOther)
		{
			return (x == aOther.x && y == aOther.y && z == aOther.z);
		}

		inline bool operator!=(C_Vector3 aOther)
		{
			return (x != aOther.x && y != aOther.y && z != aOther.z);
		}

		inline C_Vector3 operator+=(C_Vector3 aOther)
		{
			x += aOther.x;
			y += aOther.y;
			z += aOther.z;
			return *this;
		}

		inline C_Vector3 operator-=(C_Vector3 aOther)
		{
			x -= aOther.x;
			y -= aOther.y;
			z -= aOther.z;
			return *this;
		}

		inline C_Vector3 operator*=(C_Vector3 aOther)
		{
			x *= aOther.x;
			y *= aOther.y;
			z *= aOther.z;
			return *this;
		}

		inline C_Vector3 operator/=(C_Vector3 aOther)
		{
			x /= aOther.x;
			y /= aOther.y;
			z /= aOther.z;
			return *this;
		}
		
		inline static C_Vector3 random(C_Vector3 aMin, C_Vector3 aMax)
		{
			C_Vector3 ret;
			ret.x = aMin.x + (float)(rand()) / ((float)(RAND_MAX / (aMax.x - aMin.x)));
			ret.y = aMin.y + (float)(rand()) / ((float)(RAND_MAX / (aMax.y - aMin.y)));
			ret.z = aMin.z + (float)(rand()) / ((float)(RAND_MAX / (aMax.z - aMin.z)));
			return ret;
		}
		
		inline C_Vector3 normalize()
		{
			float l = sqrt((x * x) + (y * y) + (z * z));
			x /= l;
			y /= l;
			z /= l;
		}

		inline ~C_Vector3() {}
	};

	typedef C_Vector3 vec3;

}
