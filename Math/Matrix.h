#pragma once

#include <Math/Vector4.h>
#include <System/Assert.h>
#include <Core/Templates/Copy.h>

namespace Columbus
{

	class Matrix
	{
	public:
		float M[4][4];
	public:
		/*
		* Constructor
		* @param float Diag: Specifies matrix main diagonal value
		*/
		inline explicit Matrix(float Diag = 1.0f)
		{
			M[0][0] = Diag; M[0][1] = 0.0f; M[0][2] = 0.0f; M[0][3] = 0.0f;
			M[1][0] = 0.0f; M[1][1] = Diag; M[1][2] = 0.0f; M[1][3] = 0.0f;
			M[2][0] = 0.0f; M[2][1] = 0.0f; M[2][2] = Diag; M[2][3] = 0.0f;
			M[3][0] = 0.0f; M[3][1] = 0.0f; M[3][2] = 0.0f; M[3][3] = Diag;
		}
		/*
		* Constructor
		* @params Vector4 A, B, C and D specifies matrix rows
		*/
		inline Matrix(Vector4 A, Vector4 B, Vector4 C, Vector4 D)
		{
			SetRow(0, A);
			SetRow(1, B);
			SetRow(2, C);
			SetRow(3, D);
		}

		/*
		* Set this matrix to identity
		*/
		inline void SetIdentity()
		{
			M[0][0] = 1.0f; M[0][1] = 0.0f; M[0][2] = 0.0f; M[0][3] = 0.0f;
			M[1][0] = 0.0f; M[1][1] = 1.0f; M[1][2] = 0.0f; M[1][3] = 0.0f;
			M[2][0] = 0.0f; M[2][1] = 0.0f; M[2][2] = 1.0f; M[2][3] = 0.0f;
			M[3][0] = 0.0f; M[3][1] = 0.0f; M[3][2] = 0.0f; M[3][3] = 1.0f;	
		}
		/*
		* Set indexed row of this matrix
		* If Index >= 4, occures assertation
		*/
		inline void SetRow(uint32 Index, Vector4 Row)
		{
			COLUMBUS_ASSERT_MESSAGE(Index < 4, "Matrix::SetRow(): Index >= 4");

			M[Index][0] = Row.x;
			M[Index][1] = Row.y;
			M[Index][2] = Row.z;
			M[Index][3] = Row.w;
		}
		/*
		* Set indexed column of this matrix
		* If Index >= 4, occures assertation
		*/
		inline void SetColumn(uint32 Index, Vector4 Column)
		{
			COLUMBUS_ASSERT_MESSAGE(Index < 4, "Matrix::SetColumn(): Index >= 4");

			M[0][Index] = Column.x;
			M[1][Index] = Column.y;
			M[2][Index] = Column.z;
			M[3][Index] = Column.w;
		}
		/*
		* Return indexed row of this matrix
		* If Index >= 4, occures assertation
		*/
		inline Vector4 GetRow(uint32 Index) const
		{
			COLUMBUS_ASSERT_MESSAGE(Index < 4, "Matrix::GetRow(): Index >= 4")

			return Vector4(M[Index][0], M[Index][1], M[Index][2], M[Index][3]);
		}
		/*
		* Return indexed column of this matrix
		* If Index >= 4, occures assertation
		*/
		inline Vector4 GetColumn(uint32 Index) const
		{
			COLUMBUS_ASSERT_MESSAGE(Index < 4, "Matrix::GetColumn(): Index >= 4")

			return Vector4(M[0][Index], M[1][Index], M[2][Index], M[3][Index]);
		}
		/*
		* Get matrix elements in float* Elems
		* Be careful, size of float* Elems must be >= 16 (64 bytes)
		* If float* ELems == nullptr, does nothing
		* Use this for row-major matrix using, Direc3D for example
		*/
		inline void Elements(float* Elems) const
		{
			if (Elems != nullptr)
			{
				//16 is size of matrix (4 * 4)
				Copy(&M[0][0], &M[0][0] + 16, Elems);
			}
		}
		/*
		* Get transposed matrix elements in float* Elems
		* Be careful, size of float* Elems must be >= 16 (64 bytes)
		* If float* ELems == nullptr, does nothing
		* Use this for column-major matrix using, OpenGL for example
		*/
		inline void ElementsTransposed(float* Elems) const
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
		/*
		* Return transposed this matrix, but this still be intact
		* @return Matrix: Transposed matrix
		*/
		inline Matrix GetTransposed() const
		{
			return Matrix(GetColumn(0), GetColumn(1), GetColumn(2), GetColumn(3));
		}
		/*
		* Transpose this matrix
		* @return Matrix&: *this
		*/
		inline Matrix& Transpose()
		{
			*this = GetTransposed();
			return *this;
		}
		/*
		* Translate this matrix in Position
		* @return Matrix&: *this
		*/
		inline Matrix& Translate(Vector3 Position)
		{
			M[0][3] += Position.X;
			M[1][3] += Position.Y;
			M[2][3] += Position.Z;

			return *this;
		}
		/*
		* Rotate matrix around Axis on Angle (in degrees)
		* @return Matrix&: *this
		*/
		inline Matrix& Rotate(Vector3 Axis, float Angle)
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
		/*
		* Create rotation matrix by Euler Angles
		* @return Matrix: Result of rotation
		*/
		inline Matrix GetRotation(Vector3 EulerAngles)
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
		/*
		* Scale this matrix on Sacle
		* @return Matrix&: *this
		*/
		inline Matrix& Scale(Vector3 Scale)
		{
			M[0][0] *= Scale.X;
			M[1][1] *= Scale.Y;
			M[2][2] *= Scale.Z;

			return *this;
		}
		/*
		* Create perspective matrix from this
		* @return Matrix&: *this
		*/
		inline Matrix& Perspective(float FOV, float Aspect, float Near, float Far)
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
			M[2][3] = qn;
			M[3][2] = -1.0f;
			M[3][3] = 0.0f;

			return *this;
		}
		/*
		*
		*/
		inline Matrix& Ortho()
		{
			return *this;
		}
		/*
		* Create view matrix from this
		* @param Vector3 Position: Position of observer
		* @param Vector3 Forward: Forward-direction of observer
		* @param Vector3 Up: Up-direction of observer
		* @return Matrix&: *this
		*/
		inline Matrix& LookAt(Vector3 Position, Vector3 Center, Vector3 Up)
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
		/*
		* Get result of multiplying a Matrix to this
		* @param Matrix Other: The Matrix to multiply this by
		* @return Matrix: The result of multiplication
		*/
		inline Matrix operator*(const Matrix Other)
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
		/*
		* Get result of multiplying a Vector4 to this matrix
		* @param Vector4 Other: The Vector4 to multiply this by
		* @return Vector4: The result of mutiplication
		*/
		inline Vector4 operator*(const Vector4 Other)
		{
			Vector4 ResultVec;

			for (uint32 X = 0; X < 4; X++)
			{
				ResultVec.x += M[X][0] * Other.x;
				ResultVec.y += M[X][1] * Other.y;
				ResultVec.z += M[X][2] * Other.z;
				ResultVec.w += M[X][3] * Other.w;
			}

			return ResultVec;
		}
		/*
		* Multiply this by Other Matrix
		* @param Matrix Other: The Matrix to multiply by this
		* @return Matrix&: *this
		*/
		inline Matrix& operator*=(const Matrix Other)
		{
			*this = *this * Other;
			return *this;
		}
		/*
		* Get result of adding a Matrix to this
		* @param Matrix Other:
		* @return Matrix: The Matrix to add
		*/
		inline Matrix operator+(const Matrix Other)
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
		/*
		* Add Other Matrix to this
		* @param Matrix Other: The Matrix to add to this
		* @return Matrix: *this
		*/
		inline Matrix operator+=(const Matrix Other)
		{
			*this = *this + Other;
			return *this;
		}
		/*
		* Compare this Matrix and Other
		* @param Matrix Other: The Matrix to compare with this
		* @return bool: Bool-value of comparison
		*/
		inline bool operator==(const Matrix Other)
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
		/*
		* Compare this Matrix and Other
		* @param Matrix Other: The Matrix to compare with this
		* @return bool: Bool-value of comparison
		*/
		inline bool operator!=(const Matrix Other)
		{
			return !(*this == Other);
		}
		/*
		* Debug print of matrix
		*/
		inline void DebugPrint()
		{
			printf("---------------------------\n");

			for (uint32 X = 0; X < 4; X++)
			{
				printf("%f %f %f %f\n", M[X][0], M[X][1], M[X][2], M[X][3]);
			}

			printf("---------------------------\n");
		}
		/**/
		virtual ~Matrix() {}
	};

}





















