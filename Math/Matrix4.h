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
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <System/Assert.h>
#include <System/Random.h>
#include <System/System.h>

#include <Math/Vector3.h>
#include <Math/Vector4.h>

namespace Columbus
{

	class C_Matrix4;
	typedef C_Matrix4 mat4;

	class C_Matrix4
	{
	public:
		float mat[16];
		////////////////////////////////////////////////////////////////////////////
		//Constructor
		C_Matrix4()
		{
			for (int i = 0; i < 16; i++)
				mat[i] = 0.0;
		}
		////////////////////////////////////////////////////////////////////////////
		//Constructor
		C_Matrix4(const float aDiagonal)
		{
			for (int i = 0; i < 16; i++)
				mat[i] = 0.0;

			mat[0 + 0 * 4] = static_cast<float>(aDiagonal);
			mat[1 + 1 * 4] = static_cast<float>(aDiagonal);
			mat[2 + 2 * 4] = static_cast<float>(aDiagonal);
			mat[3 + 3 * 4] = static_cast<float>(aDiagonal);
		}
		////////////////////////////////////////////////////////////////////////////
		//Constructor
		C_Matrix4(const glm::mat4 aMat)
		{
			fromGLM(aMat);
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
		////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////
		//Multiply matrix
		C_Matrix4& multiply(const C_Matrix4 aOther)
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
		//Operator *
		friend C_Matrix4& operator*(C_Matrix4 aLeft, const C_Matrix4 aRight)
		{
			return aLeft.multiply(aRight);
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator *=
		C_Matrix4& operator*=(const C_Matrix4 aOther)
		{
			return multiply(aOther);
		}
		////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////
		//Create identity matrix
		static C_Matrix4& identity()
		{
			return C_Matrix4(1.0);
		}
		////////////////////////////////////////////////////////////////////////////
		//Create orthographics matrix
		static C_Matrix4& orthographic(const float aLeft, const float aRight,
			const float aBottom, const float aTop, const float aNear, const float aFar)
		{
			C_Matrix4 res(1.0);

			res.mat[0 + 0 * 4] = 2.0 / (aRight - aLeft);
			res.mat[1 + 1 * 4] = 2.0 / (aTop - aBottom);
			res.mat[2 + 2 * 4] = 2.0 / (aNear - aFar);

			res.mat[0 + 3 * 4] = (aLeft + aRight) / (aLeft - aRight);
			res.mat[1 + 3 * 4] = (aBottom + aTop) / (aBottom - aTop);
			res.mat[2 + 3 * 4] = (aFar + aNear) / (aFar - aNear);

			return res;
		}
		////////////////////////////////////////////////////////////////////////////
		//Create perspective matrix
		static C_Matrix4& perspective(const float aFOV, const float aAspect,
			const float aNear, const float aFar)
		{
			C_Matrix4 res(1.0);

			float q = 1.0 / tan(C_DegToRads(aFOV * 0.5));
			float a = q / aAspect;
			float b = (aNear + aFar) / (aNear - aFar);
			float c = (2.0 * aNear * aFar) / (aNear - aFar);

			res.mat[0 + 0 * 4] = a;
			res.mat[1 + 1 * 4] = q;
			res.mat[2 + 2 * 4] = b;
			res.mat[3 + 2 * 4] = -1.0;
			res.mat[2 + 3 * 4] = c;

			return res;
		}
		////////////////////////////////////////////////////////////////////////////
		//Translate the matrix
		static C_Matrix4& translate(C_Matrix4 aMat, const C_Vector3 aTranslation)
		{
			aMat.mat[0 + 3 * 4] = aTranslation.x;
			aMat.mat[1 + 3 * 4] = aTranslation.y;
			aMat.mat[2 + 3 * 4] = aTranslation.z;

			return aMat;
		}
		////////////////////////////////////////////////////////////////////////////
		//Scale the matrix
		static C_Matrix4& scale(C_Matrix4 aMat, const C_Vector3 aScale)
		{
			aMat.mat[0 + 0 * 4] = aScale.x;
			aMat.mat[1 + 1 * 4] = aScale.y;
			aMat.mat[2 + 2 * 4] = aScale.z;

			return aMat;
		}
		////////////////////////////////////////////////////////////////////////////
		//Translate the matrix
		C_Matrix4& translate(const C_Vector3 aTranslation)
		{
			C_Matrix4 matrix(1.0);

			matrix.mat[0 + 3 * 4] = aTranslation.x;
			matrix.mat[1 + 3 * 4] = aTranslation.y;
			matrix.mat[2 + 3 * 4] = aTranslation.z;

			*this *= matrix;

			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Rotate the matrix
		C_Matrix4& rotate(const C_Vector3 aAxis, const float aAngle)
		{
			C_Matrix4 matrix(1.0);
/*
			float r = C_DegToRads(aAngle);
			float c = cos(r);
			float s = sin(r);
			float omc = 1.0 - c;

			float x = aAxis.x;
			float y = aAxis.y;
			float z = aAxis.z;

			matrix.mat[0 + 0 * 4] = x * omc + c;
			matrix.mat[1 + 0 * 4] = y * x * omc + z * s;
			matrix.mat[2 + 0 * 4] = x * z * omc - y * s;

			matrix.mat[0 + 1 * 4] = x * y * omc - z * s;
			matrix.mat[1 + 1 * 4] = y * omc + c;
			matrix.mat[2 + 1 * 4] = y * z * omc + x * s;

			matrix.mat[0 + 2 * 4] = x * z * omc + y * s;
			matrix.mat[1 + 2 * 4] = y * z * omc - x * s;
			matrix.mat[2 + 2 * 4] = z * omc + c;*/

			

			*this *= matrix;

			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Scale the matrix
		C_Matrix4& scale(const C_Vector3 aScale)
		{
			C_Matrix4 matrix(1.0);

			matrix.mat[0 + 0 * 4] = aScale.x;
			matrix.mat[1 + 1 * 4] = aScale.y;
			matrix.mat[2 + 2 * 4] = aScale.z;

			*this *= matrix;
			
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////
		//Destructor
		~C_Matrix4() {}
	};

}









