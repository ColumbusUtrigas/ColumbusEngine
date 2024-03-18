#pragma once

#include <Math/Vector3.h>
#include <Math/Vector4.h>
#include <Core/Assert.h>
#include <cstring>
#include <cstdio>

namespace Columbus
{

	struct Matrix3x3
	{
	public:
		float M[3][3];
	public:
		float GetDeterminant() const
		{
			return
				M[0][0] * (M[1][1]*M[2][2] - M[1][2]*M[2][1]) - // cofactor of M[0][0]
				M[1][0] * (M[0][1]*M[2][2] - M[0][2]*M[2][1]) + // cofactor of M[1][0]
				M[2][0] * (M[0][1]*M[1][2] - M[0][2]*M[1][1]);  // cofactor of M[2][0]
		}

		void DebugPrint()
		{
			printf("---------------------------\n");

			for (int i = 0; i < 3; i++)
			{
				printf("%f %f %f\n", M[i][0], M[i][1], M[i][2]);
			}

			printf("---------------------------\n");
		}
	};

	class Matrix
	{
	public:
		union
		{
			float M[4][4];
			float M16[16];
			Vector4 Rows[4]; // row-major order
		};
	public:
		explicit Matrix(float Diag = 1.0f)
		{
			M[0][0] = Diag; M[0][1] = 0.0f; M[0][2] = 0.0f; M[0][3] = 0.0f;
			M[1][0] = 0.0f; M[1][1] = Diag; M[1][2] = 0.0f; M[1][3] = 0.0f;
			M[2][0] = 0.0f; M[2][1] = 0.0f; M[2][2] = Diag; M[2][3] = 0.0f;
			M[3][0] = 0.0f; M[3][1] = 0.0f; M[3][2] = 0.0f; M[3][3] = Diag;
		}

		Matrix(const Matrix& Other)
		{
			memcpy(M, Other.M, sizeof(float) * 16);
		}

		Matrix& operator=(Matrix Other)
		{
			memcpy(M, Other.M, sizeof(float) * 16);
			return *this;
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

		Vector3 ExtractTranslate() const
		{
			return Vector3(M[0][3], M[1][3], M[2][3]);
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

		// determinant of a basis, first 3 rows/columns
		float GetDeterminant3x3Basis() const
		{
			return
				M[0][0] * (M[1][1]*M[2][2] - M[1][2]*M[2][1]) - // cofactor of M[0][0]
				M[1][0] * (M[0][1]*M[2][2] - M[0][2]*M[2][1]) + // cofactor of M[1][0]
				M[2][0] * (M[0][1]*M[1][2] - M[0][2]*M[1][1]);  // cofactor of M[2][0]
		}

		// minor matrix of element M[Y][X], 3x3 matrix with X column and Y row excluded
		Matrix3x3 GetMinorMatrix(int X, int Y) const
		{
			Matrix3x3 Minor;
			int CurX = 0;
			int CurY = 0;

			for (int i = 0; i < 4; i++)
			{
				if (i != Y) // exclude Y row
				{
					CurX = 0;
					for (int j = 0; j < 4; j++)
					{
						if (j != X) // exclude X column
						{
							Minor.M[CurY][CurX] = M[i][j];
							CurX++;
						}
					}
					CurY++;
				}
			}

			return Minor;
		}

		// minor of element M[Y][X], determinant of a 3x3 matrix with X column and Y row excluded
		float GetMinor(int X, int Y) const
		{
			return GetMinorMatrix(X, Y).GetDeterminant();
		}

		float GetDeterminant() const
		{
			float det_00 = // determinant of minor M[0][0]
				M[1][1] * (M[2][2]*M[3][3] - M[2][3]*M[3][2]) - // cofactor of M[1][1]
				M[2][1] * (M[1][2]*M[3][3] - M[1][3]*M[3][2]) + // cofactor of M[2][1]
				M[3][1] * (M[1][2]*M[2][3] - M[1][3]*M[2][2]);  // cofactor of M[3][1]

			float det_10 = // determinant of minor M[1][0]
				M[0][1] * (M[2][2]*M[3][3] - M[2][3]*M[3][2]) - // cofactor of M[0][1]
				M[2][1] * (M[0][2]*M[3][3] - M[0][3]*M[3][2]) + // cofactor of M[2][1]
				M[3][1] * (M[0][2]*M[2][3] - M[0][3]*M[2][2]);  // cofactor of M[3][1]

			float det_20 = // determinant of minor M[2][0]
				M[0][1] * (M[1][2]*M[3][3] - M[1][3]*M[3][2]) - // cofactor of M[0][1]
				M[1][1] * (M[0][2]*M[3][3] - M[0][3]*M[3][2]) + // cofactor of M[1][1]
				M[3][1] * (M[0][2]*M[1][3] - M[0][3]*M[1][2]);  // cofactor of M[3][1]

			float det_30 = // determinant of minor M[3][0]
				M[0][1] * (M[1][2]*M[2][3] - M[1][3]*M[2][2]) - // cofactor of M[0][1]
				M[1][1] * (M[0][2]*M[2][3] - M[0][3]*M[2][2]) + // cofactor of M[1][1]
				M[2][1] * (M[0][2]*M[1][3] - M[0][3]*M[1][2]);  // cofactor of M[2][1]

			return
				M[0][0] * det_00 - // cofactor of M[0][0]
				M[1][0] * det_10 + // cofactor of M[1][0]
				M[2][0] * det_20 - // cofactor of M[2][0]
				M[3][0] * det_30;  // cofactor of M[3][0]
		}

		Matrix GetInverted() const
		{
			float det = GetDeterminant();

			if (abs(det) < 0.0001f)
			{
				return Matrix(1);
			}

			Matrix Result(0);

			// Compute matrix of cofactors
			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					float mul = powf(-1, (float)(i+j)); // "checkerboard" of signs for cofactors
					float minor = GetMinor(j, i) * mul;
					Result.M[i][j] = minor;
				}
			}

			// Convert it to an adjoint matrix
			Result.Transpose();

			return Result * (1.0f / det);
		}

		Matrix& Invert()
		{
			return *this = GetInverted();
		}

		// normalises basis, but leaves homogenous translation as is
		Matrix& OrthoNormalise()
		{
			SetColumn(0, GetColumn(0).Normalized());
			SetColumn(1, GetColumn(1).Normalized());
			SetColumn(2, GetColumn(2).Normalized());

			return *this;
		}

		// input must be normalised, input vector will become Y
		Matrix& OrthoNormalBasisFromVector(const Vector3& Vec)
		{
			// Using right-hand coord
			const Vector3 up = fabs(Vec.Y) < 0.999 ? Vector3(0, 1, 0) : Vector3(1, 0, 0);
			const Vector3 xAxis = Vector3::Cross(up, Vec).Normalized();
			const Vector3 yAxis = Vec;
			const Vector3 zAxis = Vector3::Cross(Vec, xAxis);

			SetColumn(0, Vector4(xAxis, 0));
			SetColumn(1, Vector4(yAxis, 0));
			SetColumn(2, Vector4(zAxis, 0));
			SetColumn(3, Vector4(0, 0, 0, 1));

			return *this;
		}

		void DecomposeTransform(Vector3& OutTranslation, Vector3& OutEulerRotationDegrees, Vector3& OutScale) const
		{
			Matrix Tmp = *this;

			// scale is a length of basis vectors
			OutScale.X = Tmp.GetColumn(0).XYZ().Length();
			OutScale.Y = Tmp.GetColumn(1).XYZ().Length();
			OutScale.Z = Tmp.GetColumn(2).XYZ().Length();

			Tmp.OrthoNormalise();

			OutEulerRotationDegrees.X = Math::Degrees(atan2f(Tmp.M[1][2], Tmp.M[2][2]));
			OutEulerRotationDegrees.Y = Math::Degrees(atan2f(-Tmp.M[0][2], sqrtf(Tmp.M[1][2] * Tmp.M[1][2] + Tmp.M[2][2] * Tmp.M[2][2])));
			OutEulerRotationDegrees.Z = Math::Degrees(atan2f(Tmp.M[0][1], Tmp.M[0][0]));

			// homogenous translation
			OutTranslation = Tmp.GetColumn(3).XYZ();
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

		Matrix operator*(const float Other) const
		{
			Matrix Result = *this;

			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					Result.M[i][j] *= Other;
				}
			}

			return Result;
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
			float Result[4];

			for (uint32 X = 0; X < 4; X++)
			{
				Result[X] = 0.0f;
				Result[X] += M[X][0] * Other.X;
				Result[X] += M[X][1] * Other.Y;
				Result[X] += M[X][2] * Other.Z;
				Result[X] += M[X][3] * Other.W;
			}

			return Vector4(Result[0], Result[1], Result[2], Result[3]);
		}

		friend Vector4 operator*(const Vector4& Left, const Matrix& Right)
		{
			float Result[4];

			for (uint32 X = 0; X < 4; X++)
			{
				Result[X] = 0.0f;
				Result[X] += Left.X * Right.M[0][X];
				Result[X] += Left.Y * Right.M[1][X];
				Result[X] += Left.Z * Right.M[2][X];
				Result[X] += Left.W * Right.M[3][X];
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


