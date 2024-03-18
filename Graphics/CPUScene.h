#pragma once

#include <vector>
#include <Math/Vector2.h>
#include <Math/Vector3.h>
#include <Math/Vector4.h>

namespace Columbus
{

	// all geometry is indexed
	struct CPUSceneMesh
	{
		std::vector<Vector3> Vertices;
		std::vector<Vector3> Normals;
		std::vector<Vector4> Tangents; // xyz - tangent, w - sign for bitangent
		std::vector<Vector2> UV1;
		std::vector<Vector2> UV2;
		std::vector<u32> Indices;

		void CalculateTangents()
		{
			Tangents.resize(Vertices.size());

			Vector3 DeltaPos[2];
			Vector2 DeltaUV[2];
			Vector3 Tangent;
			float R;

			// iterate over triangles
			for (u32 i = 0; i < (u32)Indices.size(); i += 3)
			{
				u32 Index[3]{ Indices[i], Indices[i + 1], Indices[i + 2] };

				DeltaPos[0] = Vertices[Index[1]] - Vertices[Index[0]];
				DeltaPos[1] = Vertices[Index[2]] - Vertices[Index[0]];

				DeltaUV[0] = UV1[Index[1]] - UV1[Index[0]];
				DeltaUV[1] = UV1[Index[2]] - UV1[Index[0]];

				R = 1.0f / (DeltaUV[0].X * DeltaUV[1].Y - DeltaUV[0].Y * DeltaUV[1].X);

				Tangent = (R * (DeltaPos[0] * DeltaUV[1].Y - DeltaPos[1] * DeltaUV[0].Y)).Normalized();

				// iterate over vertices in the triangle
				for (int v = 0; v < 3; v++)
				{
					u32 IndexV = Indices[i + v];
					Vector3 Normal = Normals[IndexV];

					// Gram-Schmidt orthogonalize
					Tangent = (Tangent - Normal * Vector3::Dot(Normal, Tangent)).Normalized();

					// TODO:
					// Calculate handedness
					//float Sign = (Vector3::Dot(Vector3::Cross(Normal, Tangent), Tangent2) < 0) ? -1 : 1;

					Tangents[IndexV] = Vector4(Tangent, 1.0f);
				}
			}
		}

		// TODO: trasformation?
		// TODO: this one and a GPU one should represent a shared resource of a mesh, not an instance
	};

	struct CPUScene
	{
		// 1 to 1 correspondance with GPUScene
		std::vector<CPUSceneMesh> Meshes;
	};

}
