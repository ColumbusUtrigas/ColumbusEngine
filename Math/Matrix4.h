/************************************************
*                   Matrix4.h                   *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   06.11.2017                  *
*************************************************/

#pragma once

#include <cmath>
#include <utility>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <System/Assert.h>
#include <System/Random.h>
#include <System/System.h>

#include <Math/Vector3.h>
#include <Math/Vector4.h>

namespace Columbus
{

	class Matrix4;
	typedef Matrix4 mat4;

	class Matrix4
	{
	private:
		float mat[16];
	public:
		////////////////////////////////////////////////////////////////////////////
		Matrix4()
		{
			for (int i = 0; i < 16; i++)
				mat[i] = 0.0;
		}
		////////////////////////////////////////////////////////////////////////////
		Matrix4(const float aDiagonal)
		{
			for (int i = 0; i < 16; i++)
				mat[i] = 0.0;

			mat[0 + 0 * 4] = static_cast<float>(aDiagonal);
			mat[1 + 1 * 4] = static_cast<float>(aDiagonal);
			mat[2 + 2 * 4] = static_cast<float>(aDiagonal);
			mat[3 + 3 * 4] = static_cast<float>(aDiagonal);
		}
		////////////////////////////////////////////////////////////////////////////
		Matrix4(const glm::mat4 aMat)
		{
			fromGLM(aMat);
		}
		////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////
		//Set matrix row
		void setRow(const unsigned int index, const Vector4 aRow)
		{
			COLUMBUS_ASSERT(index < 4);
			mat[index + 0 * 4] = aRow.x;
			mat[index + 1 * 4] = aRow.y;
			mat[index + 2 * 4] = aRow.z;
			mat[index + 3 * 4] = aRow.w;
		}
		////////////////////////////////////////////////////////////////////////////
		//Set matrix column
		void setColumn(const unsigned int index, const Vector4 aColumnn)
		{
			COLUMBUS_ASSERT(index < 4);
			mat[0 + index * 4] = aColumnn.x;
			mat[1 + index * 4] = aColumnn.y;
			mat[2 + index * 4] = aColumnn.z;
			mat[3 + index * 4] = aColumnn.w;
		}
		////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////
		//Conversion from GLM mat4
		void fromGLM(const glm::mat4 aMat)
		{
			float* vec = const_cast<float*>(glm::value_ptr(aMat));
			for (int i = 0; i < 16; i++)
				mat[i] = vec[i];
		}
		////////////////////////////////////////////////////////////////////////////
		//Return values array
		float* elements()
		{
			return mat;
		}
		////////////////////////////////////////////////////////////////////////////
		//Transpose matrix
		Matrix4& transpose()
		{
			std::swap(mat[1], mat[4]);
			std::swap(mat[2], mat[8]);
			std::swap(mat[3], mat[12]);
			std::swap(mat[6], mat[9]);
			std::swap(mat[7], mat[13]);
			std::swap(mat[11], mat[14]);

			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Invert matrix
		Matrix4& invert()
		{
			float tmp;
			tmp = mat[1];  mat[1] = mat[4];  mat[4] = tmp;
			tmp = mat[2];  mat[2] = mat[8];  mat[8] = tmp;
			tmp = mat[6];  mat[6] = mat[9];  mat[9] = tmp;

			float x = mat[12];
			float y = mat[13];
			float z = mat[14];
			mat[12] = -(mat[0] * x + mat[4] * y + mat[8] * z);
			mat[13] = -(mat[1] * x + mat[5] * y + mat[9] * z);
			mat[14] = -(mat[2] * x + mat[6] * y + mat[10] * z);

			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////
		//Multiply matrix
		Matrix4& multiply(const Matrix4 aOther)
		{
			for (int row = 0; row < 4; row++)
			{
				for (int col = 0; col < 4; col++)
				{
					float sum = 0.0;
					for (int e = 0; e < 4; e++)
					{
						sum += mat[row + e * 4] * aOther.mat[e + col * 4];
					}
					mat[row + col * 4] = sum;
				}
			}
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////
		//Operator =
		Matrix4& operator=(Matrix4 aOther)
		{
			for (int i = 0; i < 16; i++)
				mat[i] = aOther.mat[i];

			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator *
		friend Matrix4& operator*(Matrix4 aLeft, const Matrix4 aRight)
		{
			return aLeft.multiply(aRight);
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator *=
		Matrix4& operator*=(const Matrix4 aOther)
		{
			return multiply(aOther);
		}
		////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////
		//Create identity matrix
		Matrix4 identity()
		{
			*this = Matrix4(1.0);
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Create orthographics matrix
		Matrix4 orthographic(const float aLeft, const float aRight,
			const float aBottom, const float aTop, const float aNear, const float aFar)
		{

			mat[0 + 0 * 4] = 2.0f / (aRight - aLeft);
			mat[1 + 1 * 4] = 2.0f / (aTop - aBottom);
			mat[2 + 2 * 4] = 2.0f / (aNear - aFar);

			mat[0 + 3 * 4] = (aLeft + aRight) / (aLeft - aRight);
			mat[1 + 3 * 4] = (aBottom + aTop) / (aBottom - aTop);
			mat[2 + 3 * 4] = (aFar + aNear) / (aFar - aNear);

			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Create perspective matrix
		Matrix4 perspective(const float aFOV, const float aAspect,
			const float aNear, const float aFar)
		{
			for (int i = 0; i < 16; i++)
				mat[i] = 0.0;

			float xymax = aNear * tan(aFOV / 360.0f * 3.141592f);
			float ymin = -xymax;
			float xmin = -xymax;

			float width = xymax - xmin;
			float height = xymax - ymin;

			float depth = aFar - aNear;
			float q = -(aFar + aNear) / depth;
			float qn = -2 * (aFar * aNear) / depth;

			float w = 2 * aNear / width;
			w = w / aAspect;
			float h = 2 * aNear / height;

			mat[0] = w;
			mat[5] = h;
			mat[10] = q;
			mat[11] = -1;
			mat[14] = qn;
			mat[15] = 0;

			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Create view matrix
		Matrix4 lookAt(Vector3 aPos, Vector3 aRef, Vector3 aUp)
		{
			/*vec3 f((aRef - aPos).normalize());
			vec3 s(vec3::cross(aUp, f).normalize());
			vec3 u(vec3::cross(f, s));*/

			vec3 zaxis = (aRef - aPos).normalize();
			vec3 xaxis = vec3::cross(aUp, zaxis).normalize();
			vec3 yaxis = vec3::cross(zaxis, xaxis);

			setColumn(0, vec4(xaxis.x, yaxis.x, zaxis.x, 0.0));
			setColumn(1, vec4(xaxis.y, yaxis.y, zaxis.y, 0.0));
			setColumn(2, vec4(xaxis.z, yaxis.z, zaxis.z, 0.0));
			setColumn(3, vec4(-vec3::dot(xaxis, aPos), -vec3::dot(yaxis, aPos), -vec3::dot(zaxis, aPos), 1.0));

			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////
		Matrix4 translate(Vector3 aTranslate)
		{
			return translate(aTranslate.x, aTranslate.y, aTranslate.z);
		}
		////////////////////////////////////////////////////////////////////////////
		Matrix4 translate(const float x, const float y, const float z)
		{
			mat[0] += mat[3] * x;   mat[4] += mat[7] * x;   mat[8] += mat[11] * x;   mat[12] += mat[15] * x;
			mat[1] += mat[3] * y;   mat[5] += mat[7] * y;   mat[9] += mat[11] * y;   mat[13] += mat[15] * y;
			mat[2] += mat[3] * z;   mat[6] += mat[7] * z;   mat[10] += mat[11] * z;   mat[14] += mat[15] * z;

			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		Matrix4 rotate(Vector3 aAxis, const float aAngle)
		{
			return rotate(aAxis.x, aAxis.y, aAxis.z, aAngle);
		}
		////////////////////////////////////////////////////////////////////////////
		Matrix4 rotate(const float x, const float y, const float z, const float angle)
		{
			float c = cosf(Radians(angle));
			float s = sinf(Radians(angle));
			float c1 = 1.0f - c;
			float m0 = mat[0], m4 = mat[4], m8 = mat[8], m12 = mat[12],
			      m1 = mat[1], m5 = mat[5], m9 = mat[9], m13 = mat[13],
			      m2 = mat[2], m6 = mat[6], m10 = mat[10], m14 = mat[14];

			float r0 = x * x * c1 + c;
			float r1 = x * y * c1 + z * s;
			float r2 = x * z * c1 - y * s;
			float r4 = x * y * c1 - z * s;
			float r5 = y * y * c1 + c;
			float r6 = y * z * c1 + x * s;
			float r8 = x * z * c1 + y * s;
			float r9 = y * z * c1 - x * s;
			float r10 = z * z * c1 + c;

			mat[0] = r0 * m0 + r4 * m1 + r8 * m2;
			mat[1] = r1 * m0 + r5 * m1 + r9 * m2;
			mat[2] = r2 * m0 + r6 * m1 + r10* m2;
			mat[4] = r0 * m4 + r4 * m5 + r8 * m6;
			mat[5] = r1 * m4 + r5 * m5 + r9 * m6;
			mat[6] = r2 * m4 + r6 * m5 + r10* m6;
			mat[8] = r0 * m8 + r4 * m9 + r8 * m10;
			mat[9] = r1 * m8 + r5 * m9 + r9 * m10;
			mat[10] = r2 * m8 + r6 * m9 + r10* m10;
			mat[12] = r0 * m12 + r4 * m13 + r8 * m14;
			mat[13] = r1 * m12 + r5 * m13 + r9 * m14;
			mat[14] = r2 * m12 + r6 * m13 + r10* m14;

			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		Matrix4 scale(Vector3 aScale)
		{
			return scale(aScale.x, aScale.y, aScale.z);
		}
		////////////////////////////////////////////////////////////////////////////
		Matrix4 scale(const float x, const float y, const float z)
		{
			mat[0] *= x;   mat[4] *= x;   mat[8] *= x;   mat[12] *= x;
			mat[1] *= y;   mat[5] *= y;   mat[9] *= y;   mat[13] *= y;
			mat[2] *= z;   mat[6] *= z;   mat[10] *= z;   mat[14] *= z;

			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////
		//Destructor
		~Matrix4() {}
	};

}









