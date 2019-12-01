#pragma once

#include <Math/Vector3.h>
#include <Math/Vector4.h>
#include <Core/Assert.h>
#include <cstring>
#include <cstdio>

namespace Columbus
{

	class Matrix
	{
	public:
		float M[4][4];
	public:
		explicit Matrix(float Diag = 1.0f)
		{
			M[0][0] = Diag; M[0][1] = 0.0f; M[0][2] = 0.0f; M[0][3] = 0.0f;
			M[1][0] = 0.0f; M[1][1] = Diag; M[1][2] = 0.0f; M[1][3] = 0.0f;
			M[2][0] = 0.0f; M[2][1] = 0.0f; M[2][2] = Diag; M[2][3] = 0.0f;
			M[3][0] = 0.0f; M[3][1] = 0.0f; M[3][2] = 0.0f; M[3][3] = Diag;
		}
		
		Matrix(const Vector4& A, const Vector4& B, const Vector4& C, const Vector4& D)
		{
			SetRow(0, A);
			SetRow(1, B);
			SetRow(2, C);
			SetRow(3, D);
		}

		void SetIdentity()
		{
			M[0][0] = 1.0f; M[0][1] = 0.0f; M[0][2] = 0.0f; M[0][3] = 0.0f;
			M[1][0] = 0.0f; M[1][1] = 1.0f; M[1][2] = 0.0f; M[1][3] = 0.0f;
			M[2][0] = 0.0f; M[2][1] = 0.0f; M[2][2] = 1.0f; M[2][3] = 0.0f;
			M[3][0] = 0.0f; M[3][1] = 0.0f; M[3][2] = 0.0f; M[3][3] = 1.0f;	
		}
		
		void SetRow(uint32 Index, const Vector4& Row)
		{
			//COLUMBUS_ASSERT_MESSAGE(Index < 4, "Matrix::SetRow(): Index >= 4");

			M[Index][0] = Row.X;
			M[Index][1] = Row.Y;
			M[Index][2] = Row.Z;
			M[Index][3] = Row.W;
		}
		
		void SetColumn(uint32 Index, const Vector4& Column)
		{
			//COLUMBUS_ASSERT_MESSAGE(Index < 4, "Matrix::SetColumn(): Index >= 4");

			M[0][Index] = Column.X;
			M[1][Index] = Column.Y;
			M[2][Index] = Column.Z;
			M[3][Index] = Column.W;
		}
		
		Vector4 GetRow(uint32 Index) const
		{
			//COLUMBUS_ASSERT_MESSAGE(Index < 4, "Matrix::GetRow(): Index >= 4")

			return Vector4(M[Index][0], M[Index][1], M[Index][2], M[Index][3]);
		}
		
		Vector4 GetColumn(uint32 Index) const
		{
			//COLUMBUS_ASSERT_MESSAGE(Index < 4, "Matrix::GetColumn(): Index >= 4")

			return Vector4(M[0][Index], M[1][Index], M[2][Index], M[3][Index]);
		}
		
		void Elements(float* Elems) const
		{
			if (Elems != nullptr)
			{
				//16 is size of matrix (4 * 4)
				//std::copy(&M[0][0], &M[0][0] + 16, Elems);
				memcpy(Elems, &M[0][0], sizeof(M));
			}
		}
		
		void ElementsTransposed(float* Elems) const
		{
			if (Elems != nullptr)
			{
				for (uint32 X = 0; X < 4; X++)
				{
					for (uint32 Y = 0; Y < 4; Y++)
					{
						Elems[Y + X * 4] = M[Y][X];
					}
				}
			}
		}
		
		Matrix GetTransposed() const
		{
			return Matrix(GetColumn(0), GetColumn(1), GetColumn(2), GetColumn(3));
		}
		
		Matrix& Transpose()
		{
			return *this = GetTransposed();
		}
		
		inline Matrix& Translate(const Vector3& Position)
		{
			M[0][3] += Position.X;
			M[1][3] += Position.Y;
			M[2][3] += Position.Z;

			return *this;
		}
		
		Matrix& Rotate(const Vector3& Axis, float Angle)
		{
			float x = Axis.X;
			float y = Axis.Y;
			float z = Axis.Z;

			float c = Math::Cos(Math::Radians(Angle));
			float s = Math::Sin(Math::Radians(Angle));
			float c1 = 1.0f - c;
			float m0 = M[0][0], m4 = M[1][0], m8 = M[2][0], m12 = M[3][0],
			      m1 = M[0][1], m5 = M[1][1], m9 = M[2][1], m13 = M[3][1],
			      m2 = M[0][2], m6 = M[1][2], m10 = M[2][2], m14 = M[3][2];

			float r0 = x * x * c1 + c;
			float r1 = x * y * c1 + z * s;
			float r2 = x * z * c1 - y * s;
			float r4 = x * y * c1 - z * s;
			float r5 = y * y * c1 + c;
			float r6 = y * z * c1 + x * s;
			float r8 = x * z * c1 + y * s;
			float r9 = y * z * c1 - x * s;
			float r10 = z * z * c1 + c;

			M[0][0] = r0 * m0 + r4 * m1 + r8 * m2;
			M[1][0] = r1 * m0 + r5 * m1 + r9 * m2;
			M[2][0] = r2 * m0 + r6 * m1 + r10* m2;
			M[0][1] = r0 * m4 + r4 * m5 + r8 * m6;
			M[1][1] = r1 * m4 + r5 * m5 + r9 * m6;
			M[2][1] = r2 * m4 + r6 * m5 + r10* m6;
			M[0][2] = r0 * m8 + r4 * m9 + r8 * m10;
			M[1][2] = r1 * m8 + r5 * m9 + r9 * m10;
			M[2][2] = r2 * m8 + r6 * m9 + r10* m10;
			M[0][3] = r0 * m12 + r4 * m13 + r8 * m14;
			M[1][3] = r1 * m12 + r5 * m13 + r9 * m14;
			M[2][3] = r2 * m12 + r6 * m13 + r10* m14;

			return *this;
		}
		
		Matrix GetRotation(const Vector3& EulerAngles)
		{
			Matrix ResultMat;

			float sr, sp, sy, cr, cp, cy;

			Math::SinCos(Math::Radians(EulerAngles.X), sy, cy);
			Math::SinCos(Math::Radians(EulerAngles.Z), sp, cp);
			Math::SinCos(Math::Radians(EulerAngles.Y), sr, cr);

			// matrix = (YAW * PITCH) * ROLL
			ResultMat.M[0][0] = cp*cy;
			ResultMat.M[0][1] = cp*sy;
			ResultMat.M[0][2] = -sp;
			ResultMat.M[1][0] = sr*sp*cy+cr*-sy;
			ResultMat.M[1][1] = sr*sp*sy+cr*cy;
			ResultMat.M[1][2] = sr*cp;
			ResultMat.M[2][0] = (cr*sp*cy+-sr*-sy);
			ResultMat.M[2][1] = (cr*sp*sy+-sr*cy);
			ResultMat.M[2][2] = cr*cp;
			ResultMat.M[3][0] = 0.0f;
			ResultMat.M[3][1] = 0.0f;
			ResultMat.M[3][2] = 0.0f;

			return ResultMat;
		}
		
		Matrix& Scale(Vector3 Scale)
		{
			M[0][0] *= Scale.X;
			M[1][1] *= Scale.Y;
			M[2][2] *= Scale.Z;

			return *this;
		}
		
		Matrix& Perspective(float FOV, float Aspect, float Near, float Far)
		{
			for (uint32 X = 0; X < 4; X++)
			{
				for (uint32 Y = 0; Y < 4; Y++)
				{
					M[X][Y] = 0.0f;
				}
			}

			float xymax = Near * tan(FOV / 360.0f * 3.141592f);
			float ymin = -xymax;
			float xmin = -xymax;

			float width = xymax - xmin;
			float height = xymax - ymin;

			float depth = Far - Near;
			float q = -(Far + Near) / depth;
			float qn = -2 * (Far * Near) / depth;

			float w = 2 * Near / width;
			w = w / Aspect;
			float h = 2 * Near / height;

			M[0][0] = w;
			M[1][1] = h;
			M[2][2] = q;
			M[3][2] = qn;
			M[2][3] = -1.0f;

			return *this;
		}
		
		Matrix& Ortho()
		{
			return *this;
		}
		
		Matrix& LookAt(const Vector3& Position, const Vector3& Center, const Vector3& Up)
		{
			Vector3 const f(Vector3::Normalize(Center - Position));
			Vector3 const s(Vector3::Normalize(Vector3::Cross(f, Up)));
			Vector3 const u(Vector3::Cross(s, f));

			SetIdentity();
			M[0][0] = s.X;
			M[1][0] = s.Y;
			M[2][0] = s.Z;
			M[0][1] = u.X;
			M[1][1] = u.Y;
			M[2][1] = u.Z;
			M[0][2] = -f.X;
			M[1][2] = -f.Y;
			M[2][2] = -f.Z;
			M[3][0] = -Vector3::Dot(s, Position);
			M[3][1] = -Vector3::Dot(u, Position);
			M[3][2] = Vector3::Dot(f, Position);
			return *this;
		}
		
		Matrix operator*(const Matrix& Other) const
		{
			Matrix ResultMat;

			for (uint32 X = 0; X < 4; X++)
			{
				for (uint32 Y = 0; Y < 4; Y++)
				{
					float Sum = 0.0f;

					for (uint32 E = 0; E < 4; E++)
					{
						Sum += M[X][E] * Other.M[E][Y];
					}

					ResultMat.M[X][Y] = Sum;
				}
			}

			return ResultMat;
		}
		
		Vector4 operator*(const Vector4& Other) const
		{
			static float Result[4];

			for (uint32 X = 0; X < 4; X++)
			{
				Result[X] = 0.0f;
				Result[X] += M[0][X] * Other.X;
				Result[X] += M[1][X] * Other.Y;
				Result[X] += M[2][X] * Other.Z;
				Result[X] += M[3][X] * Other.W;
			}

			return Vector4(Result[0], Result[1], Result[2], Result[3]);
		}

		friend Vector4 operator*(const Vector4& Left, const Matrix& Right)
		{
			static float Result[4];

			for (uint32 X = 0; X < 4; X++)
			{
				Result[X] = 0.0f;
				Result[X] += Left.X * Right.M[X][0];
				Result[X] += Left.Y * Right.M[X][1];
				Result[X] += Left.Z * Right.M[X][2];
				Result[X] += Left.W * Right.M[X][3];
			}

			return Vector4(Result[0], Result[1], Result[2], Result[3]);
		}

		Matrix& operator*=(const Matrix& Other)
		{
			*this = *this * Other;
			return *this;
		}
		
		Matrix operator+(const Matrix& Other)
		{
			Matrix ResultMat;

			for(uint32 X = 0; X < 4; X++)
			{
				for(uint32 Y = 0; Y < 4; Y++)
				{
					ResultMat.M[X][Y] = M[X][Y] + Other.M[X][Y];
				}
			}

			return ResultMat;
		}
		
		Matrix operator+=(const Matrix& Other)
		{
			*this = *this + Other;
			return *this;
		}
		
		bool operator==(const Matrix& Other)
		{
			for (uint32 X = 0; X < 4; X++)
			{
				for (uint32 Y = 0; Y < 4; Y++)
				{
					if (M[X][Y] != Other.M[X][Y])
					{
						return false;
					}
				}
			}

			return true;
		}
		
		bool operator!=(const Matrix& Other)
		{
			return !(*this == Other);
		}
		
		void DebugPrint()
		{
			printf("---------------------------\n");

			for (uint32 X = 0; X < 4; X++)
			{
				printf("%f %f %f %f\n", M[X][0], M[X][1], M[X][2], M[X][3]);
			}

			printf("---------------------------\n");
		}
	};

}


