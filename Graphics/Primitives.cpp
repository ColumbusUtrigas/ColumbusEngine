#include <Graphics/Primitives.h>
#include <cmath>

namespace Columbus
{
	
	std::vector<Vertex> PrimitivePlane(const Vector3& Size)
	{
		float verts[18] =
		{
			0.5f * Size.X, 0.0f, -0.5f * Size.Y,
			-0.5f * Size.X, 0.0f, -0.5f * Size.Y,
			-0.5f * Size.X, 0.0f, 0.5f * Size.Y,
	
			-0.5f * Size.X, 0.0f, 0.5f * Size.Y,
			0.5f * Size.X, 0.0f, 0.5f * Size.Y,
			0.5f * Size.X, 0.0f, -0.5f * Size.Y
		};

		float uvs[12] =
		{
			1.0, 1.0,
			0.0, 1.0,
			0.0, 0.0,

			0.0, 0.0,
			1.0, 0.0,
			1.0, 1.0
		};

		std::vector<Vertex> v;

		Vector3 norm(0, 1, 0);
		Vector3 tang(0, 0, -1);
		Vector3 bitang(-1, 0, 0);

		Vertex vert;

		for (size_t i = 0; i < 6; i++)
		{
			vert.pos = Vector3(verts[0 + i * 3], verts[1 + i * 3], verts[2 + i * 3]);
			vert.UV = Vector2(uvs[0 + i * 2], uvs[1 + i * 2]);
			vert.normal = norm;
			vert.tangent = tang;
			vert.bitangent = bitang;
			v.push_back(vert);
		};

		return v;
	}
	
	std::vector<Vertex> PrimitiveBox(const Vector3& Size)
	{
		Vector3 v[8] = 
		{
			Vector3(-0.5f, -0.5f, 0.5f),
			Vector3(-0.5f, 0.5f, 0.5f),
			Vector3(0.5f, -0.5f, 0.5f),
			Vector3(0.5f, 0.5f, 0.5f),

			Vector3(-0.5f, -0.5f, -0.5f),
			Vector3(-0.5f, 0.5f, -0.5f),
			Vector3(0.5f, -0.5f, -0.5f),
			Vector3(0.5f, 0.5f, -0.5f)
		};

		Vector2 u[4] =
		{
			Vector2(0.0f, 0.0f),
			Vector2(0.0f, 1.0f),
			Vector2(1.0f, 0.0f),
			Vector2(1.0f, 1.0f)
		};

		Vector3 n[6] = 
		{
			Vector3(0, 0, -1),
			Vector3(0, 0, 1),
			Vector3(1, 0, 0),
			Vector3(-1, 0, 0),
			Vector3(0, 1, 0),
			Vector3(0, -1, 0)
		};

		int vindices[36] =
		{
			0, 2, 3, 0, 3, 1,
			6, 4, 7, 4, 5, 7,
			2, 6, 7, 2, 7, 3,
			0, 1, 5, 0, 5, 4,
			1, 3, 7, 1, 7, 5,
			0, 4, 6, 0, 6, 2
		};

		int uindices[36] = 
		{
			0, 2, 3, 0, 3, 1,
			0, 2, 1, 2, 3, 1,
			0, 2, 3, 0, 3, 1,
			2, 3, 1, 2, 1, 0,
			0, 2, 3, 0, 3, 1,
			1, 0, 2, 1, 2, 3
		};

		std::vector<Vertex> verts;

		Vertex vert[3];
		Vector3 deltaPos1, deltaPos2;
		Vector2 deltaUV1, deltaUV2;
		Vector3 tangent, bitangent;
		float r;

		size_t i, j, c = 0;
		for (i = 0; i < 12; i++)
		{
			for (j = 0; j < 3; j++)
			{
				vert[j].pos = v[vindices[c]] * Size;
				vert[j].UV = u[uindices[c]];
				vert[j].normal = n[c / 6];

				c++;
			}

			deltaPos1 = vert[1].pos - vert[0].pos;
			deltaPos2 = vert[2].pos - vert[0].pos;

			deltaUV1 = vert[1].UV - vert[0].UV;
			deltaUV2 = vert[2].UV - vert[0].UV;

			r = 1.0f / (deltaUV1.X * deltaUV2.Y - deltaUV1.Y * deltaUV2.X);
			tangent = (deltaPos1 * deltaUV2.Y - deltaPos2 * deltaUV1.Y) * r;
			bitangent = (deltaPos2 * deltaUV1.X - deltaPos1 * deltaUV2.X) * r;

			for (j = 0; j < 3; j++)
			{
				vert[j].tangent = tangent;
				vert[j].bitangent = bitangent;
				verts.push_back(vert[j]);
			}
		}

		return verts;
	}
	
	std::vector<Vertex> PrimitiveSphere(float Radius, uint32 Rings, uint32 Sectors)
	{
		constexpr double PI = 3.141592653589793238462643383279502884197;

		std::vector<Vertex> verts;

		std::vector<Vector3> vertices;
		std::vector<Vector2> texcoords;
		std::vector<Vector3> normals;

		float const R = 1.0f / (float)(Rings - 1);
		float const S = 1.0f / (float)(Sectors - 1);
		size_t r, s;
		Vertex vert;

		vertices.resize(Rings * Sectors * 3);
		texcoords.resize(Rings * Sectors * 2);
		normals.resize(Rings * Sectors * 3);
		std::vector<Vector3>::iterator v = vertices.begin();
		std::vector<Vector2>::iterator t = texcoords.begin();
		std::vector<Vector3>::iterator n = normals.begin();

		float dtheta = static_cast<float>(PI / 12);
		float dphi = static_cast<float>(2 * PI / 10);
		float theta = 0.0f;
		float phi = 0.0f;

		for (r = 0; r < Rings; r++)
		{
			theta += dtheta;
			for (s = 0; s < Sectors; s++)
			{
				phi += dphi;
				float const x = sin(theta) * cos(phi);
				float const y = sin(theta) * sin(phi);
				float const z = cos(theta);

				*v++ = Vector3(x * Radius, y * Radius, z * Radius);
				*t++ = Vector2(s * S, r * R);
				*n++ = Vector3(x, y, z);
			}
		}

		for (r = 0; r < Rings - 1; r++)
		{
			for (s = 0; s < Sectors - 1; s++)
			{
				vert.pos = vertices[r * Sectors + s]; vert.UV = texcoords[r * Sectors + s]; vert.normal = normals[r * Sectors + s];
				verts.push_back(vert);
				vert.pos = vertices[(r + 1) * Sectors + s]; vert.UV = texcoords[(r + 1) * Sectors + s]; vert.normal = normals[(r + 1) * Sectors + s];
				verts.push_back(vert);
				vert.pos = vertices[(r + 1) * Sectors + (s + 1)]; vert.UV = texcoords[(r + 1) * Sectors + (s + 1)]; vert.normal = normals[(r + 1) * Sectors + (s + 1)];
				verts.push_back(vert);
				vert.pos = vertices[r * Sectors + s]; vert.UV = texcoords[r * Sectors + s]; vert.normal = normals[r * Sectors + s];
				verts.push_back(vert);
				vert.pos = vertices[(r + 1) * Sectors + (s + 1)]; vert.UV = texcoords[(r + 1) * Sectors + (s + 1)]; vert.normal = normals[(r + 1) * Sectors + (s + 1)];
				verts.push_back(vert);
				vert.pos = vertices[r * Sectors + (s + 1)]; vert.UV = texcoords[r * Sectors + (s + 1)]; vert.normal = normals[r * Sectors + (s + 1)];
			}
		}

		return verts;
	}
	
	std::vector<Vertex> PrimitiveCone(float Base, float Height, uint32 Slices, uint32 Stacks)
	{
		std::vector<Vertex> verts;

		return verts;
	}
	
	std::vector<Vertex> PrimitiveTorus(float Inner, float Outer, uint32 Sides, uint32 Rings)
	{
		std::vector<Vertex> verts;

		return verts;
	}
	
	std::vector<Vertex> PrimitiveCylinder(float Radius, float Height, uint32 Slices, uint32 Stacks)
	{
		std::vector<Vertex> verts;

		return verts;
	}

}



