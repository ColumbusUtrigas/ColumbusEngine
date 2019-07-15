#pragma once

#include <Math/Matrix.h>
#include <Math/Vector4.h>
#include <Math/Box.h>

namespace Columbus
{

	class Frustum
	{
	private:
		Vector4 Planes[6];
	public:
		Frustum() {}
		Frustum(const Matrix& ViewProjection) { SetProjection(ViewProjection); }

		void SetProjection(const Matrix& ViewProjection)
		{
			const auto M = &ViewProjection.M[0][0];

			const auto ExtractPlane = [&](Vector4& Plane, int Row)->void
			{
				int Scale = Row < 0 ? -1 : 1;
				Row = Math::Abs(Row) - 1;

				Plane.X = M[3]  + Scale * M[Row];
				Plane.Y = M[7]  + Scale * M[Row + 4];
				Plane.Z = M[11] + Scale * M[Row + 8];
				Plane.W = M[15] + Scale * M[Row + 12];

				float Length = Math::Sqrt(Plane.X * Plane.X + Plane.Y * Plane.Y + Plane.Z * Plane.Z);
				Plane /= Length;
			};

			ExtractPlane(Planes[0], +1);
			ExtractPlane(Planes[1], -1);
			ExtractPlane(Planes[2], +2);
			ExtractPlane(Planes[3], -2);
			ExtractPlane(Planes[4], +3);
			ExtractPlane(Planes[5], -3);

			//Planes[0] = Vector4(M[0][3] - M[0][1], M[1][3] - M[1][1], M[2][3] - M[2][1], -M[3][3] + M[3][1]); //Top
			//Planes[1] = Vector4(M[0][3] + M[0][1], M[1][3] + M[1][1], M[2][3] + M[2][1], -M[3][3] - M[3][1]); //Bottom
			//Planes[2] = Vector4(M[0][3] + M[0][0], M[1][3] + M[1][0], M[2][3] + M[2][0], -M[3][3] - M[3][0]); //Left
			//Planes[3] = Vector4(M[0][3] - M[0][0], M[1][3] - M[1][0], M[2][3] - M[2][0], -M[3][3] + M[3][0]); //Right
			//Planes[4] = Vector4(M[0][3] + M[0][2], M[1][3] + M[1][2], M[2][3] + M[2][2], -M[3][3] - M[3][2]); //Near
			//Planes[5] = Vector4(M[0][3] - M[0][2], M[1][3] - M[1][2], M[2][3] - M[2][2], -M[3][3] + M[3][2]); //Far

			/*for (int i = 0; i < 6; i++)
			{
				Planes[i] *= 1.0 / Math::Sqrt(Math::Pow(Planes[i].X, 2) + Math::Pow(Planes[i].Y, 2) + Math::Pow(Planes[i].Z, 2));
			}*/
		}

		bool Check(const Vector3& Point)
		{
			for (int i = 0; i < 6; i++)
			{
				float d = Planes[i].X * Point.X + Planes[i].Y * Point.Y + Planes[i].Z * Point.Z + Planes[i].W;

				if (d < 0)
				{
					return false;
				}
			}

			return true;
		}

		bool Check(const Box& AABB)
		{
			for (int i = 0; i < 6; i++)
			{
				float d = Math::Max(AABB.Min.X * Planes[i].X, AABB.Max.X * Planes[i].X)
				        + Math::Max(AABB.Min.Y * Planes[i].Y, AABB.Max.Y * Planes[i].Y)
				        + Math::Max(AABB.Min.Z * Planes[i].Z, AABB.Max.Z * Planes[i].Z)
				        + Planes[i].W;
				
				if (d < 0) return false;
			}

			return true;
		}

		/*bool Check(const Box& BoundingBox, const Matrix& ModelMatrix)
		{
			return true;
		}*/

		~Frustum() {}
	};

}


