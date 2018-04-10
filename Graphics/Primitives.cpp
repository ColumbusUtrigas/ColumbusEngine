/************************************************
*                Primitives.cpp                 *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#include <Graphics/Primitives.h>
#include <cmath>

namespace Columbus
{
	//////////////////////////////////////////////////////////////////////////////
	//Generate plane
	std::vector<Vertex> PrimitivePlane(Vector3 aSize)
	{
		float verts[18] =
		{
			0.5f * aSize.x, 0.0f, -0.5f * aSize.y,
			-0.5f * aSize.x, 0.0f, -0.5f * aSize.y,
			-0.5f * aSize.x, 0.0f, 0.5f * aSize.y,
	
			-0.5f * aSize.x, 0.0f, 0.5f * aSize.y,
			0.5f * aSize.x, 0.0f, 0.5f * aSize.y,
			0.5f * aSize.x, 0.0f, -0.5f * aSize.y
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

		vec3 norm(0, 1, 0);
		vec3 tang(0, 0, -1);
		vec3 bitang(-1, 0, 0);

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
	//////////////////////////////////////////////////////////////////////////////
	//Generate box
	std::vector<Vertex> PrimitiveBox(Vector3 aSize)
	{
		vec3 v[8] = 
		{
			vec3(-0.5f, -0.5f, 0.5f),
			vec3(-0.5f, 0.5f, 0.5f),
			vec3(0.5f, -0.5f, 0.5f),
			vec3(0.5f, 0.5f, 0.5f),

			vec3(-0.5f, -0.5f, -0.5f),
			vec3(-0.5f, 0.5f, -0.5f),
			vec3(0.5f, -0.5f, -0.5f),
			vec3(0.5f, 0.5f, -0.5f)
		};

		vec2 u[4] =
		{
			vec2(0.0f, 0.0f),
			vec2(0.0f, 1.0f),
			vec2(1.0f, 0.0f),
			vec2(1.0f, 1.0f)
		};

		vec3 n[6] = 
		{
			vec3(0, 0, -1),
			vec3(0, 0, 1),
			vec3(1, 0, 0),
			vec3(-1, 0, 0),
			vec3(0, 1, 0),
			vec3(0, -1, 0)
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
		vec3 deltaPos1, deltaPos2;
		vec2 deltaUV1, deltaUV2;
		vec3 tangent, bitangent;
		float r;

		size_t i, j, c = 0;
		for (i = 0; i < 12; i++)
		{
			for (j = 0; j < 3; j++)
			{
				vert[j].pos = v[vindices[c]] * aSize;
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
	//////////////////////////////////////////////////////////////////////////////
	//Generate sphere
	std::vector<Vertex> PrimitiveSphere(float aRadius, size_t aRings, size_t aSectors)
	{
		const double PI = 3.141592653589793238462643383279502884197;

		std::vector<Vertex> verts;

		std::vector<vec3> vertices;
		std::vector<vec2> texcoords;
		std::vector<vec3> normals;

		float const R = 1.0f / static_cast<float>(aRings - 1);
		float const S = 1.0f / static_cast<float>(aSectors - 1);
		size_t r, s;
		Vertex vert;

		vertices.resize(aRings * aSectors * 3);
		texcoords.resize(aRings * aSectors * 2);
		normals.resize(aRings * aSectors * 3);
		std::vector<vec3>::iterator v = vertices.begin();
		std::vector<vec2>::iterator t = texcoords.begin();
		std::vector<vec3>::iterator n = normals.begin();

		float dtheta = static_cast<float>(PI / 12);
		float dphi = static_cast<float>(2 * PI / 10);
		float theta = 0.0f;
		float phi = 0.0f;

		for (r = 0; r < aRings; r++)
		{
			theta += dtheta;
			for (s = 0; s < aSectors; s++)
			{
				phi += dphi;
				float const x = sin(theta) * cos(phi);
				float const y = sin(theta) * sin(phi);
				float const z = cos(theta);

				*v++ = vec3(x * aRadius, y * aRadius, z * aRadius);
				*t++ = vec2(s * S, r * R);
				*n++ = vec3(x, y, z);
			}
		}

		for (r = 0; r < aRings - 1; r++)
		{
			for (s = 0; s < aSectors - 1; s++)
			{
				vert.pos = vertices[r * aSectors + s]; vert.UV = texcoords[r * aSectors + s]; vert.normal = normals[r * aSectors + s];
				verts.push_back(vert);
				vert.pos = vertices[(r + 1) * aSectors + s]; vert.UV = texcoords[(r + 1) * aSectors + s]; vert.normal = normals[(r + 1) * aSectors + s];
				verts.push_back(vert);
				vert.pos = vertices[(r + 1) * aSectors + (s + 1)]; vert.UV = texcoords[(r + 1) * aSectors + (s + 1)]; vert.normal = normals[(r + 1) * aSectors + (s + 1)];
				verts.push_back(vert);
				vert.pos = vertices[r * aSectors + s]; vert.UV = texcoords[r * aSectors + s]; vert.normal = normals[r * aSectors + s];
				verts.push_back(vert);
				vert.pos = vertices[(r + 1) * aSectors + (s + 1)]; vert.UV = texcoords[(r + 1) * aSectors + (s + 1)]; vert.normal = normals[(r + 1) * aSectors + (s + 1)];
				verts.push_back(vert);
				vert.pos = vertices[r * aSectors + (s + 1)]; vert.UV = texcoords[r * aSectors + (s + 1)]; vert.normal = normals[r * aSectors + (s + 1)];
			}
		}

		return verts;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Generate cone
	std::vector<Vertex> PrimitiveCone(const float aBase, const float aHeight, const size_t aSlices, const size_t aStacks)
	{
		std::vector<Vertex> verts;

		return verts;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Generate torus
	std::vector<Vertex> PrimitiveTorus(const float aInner, const float aOuter, const size_t aSides, const size_t aRings)
	{
		std::vector<Vertex> verts;

		return verts;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Generate cylinder
	std::vector<Vertex> PrimitiveCylinder(const float aRadius, const float aHeight, const size_t aSlices, const size_t aStacks)
	{
		std::vector<Vertex> verts;

		return verts;
	}

}



