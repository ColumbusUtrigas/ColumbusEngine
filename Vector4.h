#pragma once

#include <cmath>
#include <glm/glm.hpp>

namespace C
{

	struct C_Vector4
	{
		float x;
		float y;
		float z;
		float w;

		C_Vector4()
		{
			x = 0;
			y = 0;
			z = 0;
			w = 0;
		}

		inline C_Vector4(const float aX, const float aY, const float aZ, const float aW)
		{
			x = (float)aX;
			y = (float)aY;
			z = (float)aZ;
			w = (float)aW;
		}

		inline C_Vector4 operator=(C_Vector4 other)
		{
			x = other.x;
			y = other.y;
			z = other.z;
			w = other.w;
			return *this;
		}

		inline friend C_Vector4& operator+(C_Vector4& left, const C_Vector4& right)
		{
			left.x += right.x;
			left.y += right.y;
			left.z += right.z;
			left.w += right.w;
			return left;
		}

		inline friend C_Vector4& operator-(C_Vector4& left, const C_Vector4& right)
		{
			left.x -= right.x;
			left.y -= right.y;
			left.z -= right.z;
			left.w -= right.w;
			return left;
		}

		inline friend C_Vector4& operator*(C_Vector4& left, const C_Vector4& right)
		{
			left.x *= right.x;
			left.y *= right.y;
			left.z *= right.z;
			left.w *= right.w;
			return left;
		}

		inline friend C_Vector4& operator*(C_Vector4& left, const float right)
		{
			C_Vector4 tmp = left;
			tmp.x *= right;
			tmp.y *= right;
			tmp.z *= right;
			tmp.w *= right;
			return left;
		}

		inline friend C_Vector4& operator/(C_Vector4& left, const C_Vector4& right)
		{
			left.x /= right.x;
			left.y /= right.y;
			left.z /= right.z;
			left.w /= right.w;
			return left;
		}

		inline bool operator==(const C_Vector4& other)
		{
			return x == other.x && y == other.y && z == other.z && w == other.w;
		}

		inline bool operator!=(const C_Vector4& other)
		{
			return x != other.x && y != other.y && z != other.z && w != other.w;
		}

		inline C_Vector4 operator+=(const C_Vector4& other)
		{
			x += other.x;
			y += other.y;
			z += other.z;
			w += other.w;
		}

		inline C_Vector4 operator-=(const C_Vector4& other)
		{
			x -= other.x;
			y -= other.y;
			z -= other.z;
			w -= other.w;
		}

		inline C_Vector4 operator*=(const C_Vector4& other)
		{
			x *= other.x;
			y *= other.y;
			z *= other.z;
			w *= other.w;
		}

		inline C_Vector4 operator/=(const C_Vector4& other)
		{
			x /= other.x;
			y /= other.y;
			z /= other.z;
			w /= other.w;
		}

		inline void normalize()
		{
			float norm = sqrt(x * x + y * y + z * z + w * w);
			if(x != 0)
				x = x / norm;
			if(y != 0)
				y = y / norm;
			if(z != 0)
				z = z / norm;
			if(w != 0)
				w = w / norm;
		}

		inline glm::vec4 toGLM()
		{
			return glm::vec4(x, y, z, w);
		}

		inline void fromGLM(glm::vec4 aVec)
		{
			x = aVec.x;
			y = aVec.y;
			z = aVec.z;
			w = aVec.w;
		}
	};

	typedef C_Vector4 vec4;

}
