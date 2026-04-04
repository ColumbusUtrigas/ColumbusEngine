#include "Mesh.h"

namespace Columbus
{
	void CPUMeshResource::CalculateNormals()
	{
		Normals.assign(Vertices.size(), Vector3(0.0f));

		for (u32 i = 0; i + 2 < (u32)Indices.size(); i += 3)
		{
			const u32 i1 = Indices[i + 0];
			const u32 i2 = Indices[i + 1];
			const u32 i3 = Indices[i + 2];

			if (i1 >= Vertices.size() || i2 >= Vertices.size() || i3 >= Vertices.size())
				continue;

			const Vector3 Edge1 = Vertices[i2] - Vertices[i1];
			const Vector3 Edge2 = Vertices[i3] - Vertices[i1];
			const Vector3 FaceNormal = Vector3::Cross(Edge1, Edge2);

			Normals[i1] += FaceNormal;
			Normals[i2] += FaceNormal;
			Normals[i3] += FaceNormal;
		}

		for (Vector3& Normal : Normals)
		{
			if (Normal.Length() > 1e-6f)
				Normal = Normal.Normalized();
			else
				Normal = Vector3(0.0f, 1.0f, 0.0f);
		}
	}

	void CPUMeshResource::CalculateTangents()
	{
		Tangents.resize(Vertices.size());

		std::vector<Vector3> tan1(Vertices.size(), Vector3(0, 0, 0));
		std::vector<Vector3> tan2(Vertices.size(), Vector3(0, 0, 0));

		// Iterate over triangles to accumulate weighted tangents

		if (!UV1.empty())
		{
			for (u32 i = 0; i < (u32)Indices.size(); i += 3)
			{
				u32 i1 = Indices[i];
				u32 i2 = Indices[i + 1];
				u32 i3 = Indices[i + 2];

				Vector3 v1 = Vertices[i1];
				Vector3 v2 = Vertices[i2];
				Vector3 v3 = Vertices[i3];

				Vector2 w1 = UV1[i1];
				Vector2 w2 = UV1[i2];
				Vector2 w3 = UV1[i3];

				float x1 = v2.X - v1.X;
				float x2 = v3.X - v1.X;
				float y1 = v2.Y - v1.Y;
				float y2 = v3.Y - v1.Y;
				float z1 = v2.Z - v1.Z;
				float z2 = v3.Z - v1.Z;

				float s1 = w2.X - w1.X;
				float s2 = w3.X - w1.X;
				float t1 = w2.Y - w1.Y;
				float t2 = w3.Y - w1.Y;

				float det = s1 * t2 - s2 * t1;

				if (fabsf(det) > 1e-6f)
				{
					float r = 1.0f / det;

					// S-Direction (Tangent)
					Vector3 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);
					// T-Direction (Bitangent/Binormal)
					Vector3 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r);

					// ACCUMULATE: Add to existing values for shared vertices
					tan1[i1] += sdir;
					tan1[i2] += sdir;
					tan1[i3] += sdir;

					tan2[i1] += tdir;
					tan2[i2] += tdir;
					tan2[i3] += tdir;
				}
			}
		}

		// Iterate over vertices to Orthogonalize and Calculate Handedness
		for (u32 a = 0; a < (u32)Vertices.size(); a++)
		{
			Vector3 n = Normals[a];
			Vector3 t = tan1[a];

			if (t.Length() < 1e-5f)
			{
				// We need a vector perpendicular to Normal.
				// If Normal is roughly pointing up Y, use Z as the helper. 
				// Otherwise use Y.
				Vector3 helper = (fabsf(n.Y) > 0.999f) ? Vector3(0, 0, 1) : Vector3(0, 1, 0);

				// Generate a valid Tangent
				t = Vector3::Cross(n, helper).Normalized();
				float w = 1.0f;
				Tangents[a] = Vector4(t, w);
			}
			else
			{
				// Gram-Schmidt Orthogonalize
				Vector3 xyz = (t - n * Vector3::Dot(n, t)).Normalized();

				// Calculate Handedness
				float w = (Vector3::Dot(Vector3::Cross(n, t), tan2[a]) < 0.0f) ? -1.0f : 1.0f;
				Tangents[a] = Vector4(xyz, w);
			}
		}
	}

}
