#pragma once

#include <cmath>
#include <glm/glm.hpp>

namespace C
{

	struct C_Vector3
	{
		float x;
		float y;
		float z;

		C_Vector3()
		{
			x = 0;
			y = 0;
			z = 0;
		}

		inline C_Vector3(const float aX, const float aY, const float aZ)
		{
			x = (float)aX;
			y = (float)aY;
			z = (float)aZ;
		}

		inline C_Vector3 operator=(C_Vector3 other)
		{
			x = other.x;
			y = other.y;
			z = other.z;
			return *this;
		}

		inline friend C_Vector3& operator+(C_Vector3& left, const C_Vector3& right)
		{
			left.x += right.x;
			left.y += right.y;
			left.z += right.z;
			return left;
		}

		inline friend C_Vector3& operator-(C_Vector3& left, const C_Vector3& right)
		{
			left.x -= right.x;
			left.y -= right.y;
			left.z -= right.z;
			return left;
		}

		inline friend C_Vector3& operator*(C_Vector3& left, const C_Vector3& right)
		{
			left.x *= right.x;
			left.y *= right.y;
			left.z *= right.z;
			return left;
		}

		inline friend C_Vector3& operator*(C_Vector3& left, const float right)
		{
			left.x *= right;
			left.y *= right;
			left.z *= right;
			return left;
		}

		inline friend C_Vector3& operator/(C_Vector3& left, const C_Vector3& right)
		{
			left.x /= right.x;
			left.y /= right.y;
			left.z /= right.z;
			return left;
		}

		inline bool operator==(const C_Vector3& other)
		{
			return x == other.x && y == other.y && z == other.z;
		}

		inline bool operator!=(const C_Vector3& other)
		{
			return x != other.x && y != other.y && z != other.z;
		}

		inline C_Vector3 operator+=(const C_Vector3& other)
		{
			x += other.x;
			y += other.y;
			z += other.z;
		}

		inline C_Vector3 operator-=(const C_Vector3& other)
		{
			x -= other.x;
			y -= other.y;
			z -= other.z;
		}

		inline C_Vector3 operator*=(const C_Vector3& other)
		{
			x *= other.x;
			y *= other.y;
			z *= other.z;
		}

		inline C_Vector3 operator/=(const C_Vector3& other)
		{
			x /= other.x;
			y /= other.y;
			z /= other.z;
		}

		inline void normalize()
		{
			float norm = sqrt(x * x + y * y + z * z);
			if(x != 0)
				x = x / norm;
			if(y != 0)
				y = y / norm;
			if(z != 0)
				z = z / norm;
		}

		inline glm::vec3 toGLM()
		{
			return glm::vec3(x, y, z);
		}

		inline void fromGLM(glm::vec3 aVec)
		{
			x = aVec.x;
			y = aVec.y;
			z = aVec.z;
		}
	};

	typedef C_Vector3 vec3;

}
