#pragma once
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

#include <Math/Vector2.h>
#include <Math/Vector3.h>
#include <Math/Vector4.h>

namespace C
{

	struct C_Vertex
	{
		C_Vector3 pos;
		C_Vector2 UV;
		C_Vector3 normal;
	};

	namespace Importer
	{

		class C_Importer
		{

		};

		inline std::vector<C_Vertex> C_LoadOBJVertices(const char* aFile)
		{
			FILE* fp = fopen(aFile, "rt");

			std::vector<C_Vertex> nil;

			if (!fp)
				return nil;

			size_t size;

			std::vector<C_Vertex> verts;
			std::vector<C_Vector3> tv;
			std::vector<C_Vector2> tu;
			std::vector<C_Vector3> tn;
			std::vector<unsigned int> tf;

			while (1)
			{
				char line[256];
				int res = fscanf(fp, "%s", line);
				if (res == EOF)
					break;

				if (strcmp(line, "o") == 0)
				{
					char* n;
				}

				if (strcmp(line, "v") == 0)
				{
					C_Vector3 ver;
					fscanf(fp, "%f %f %f\n", &ver.x, &ver.y, &ver.z);
					tv.push_back(ver);
				}

				if (strcmp(line, "vt") == 0)
				{
					C_Vector2 uv;
					fscanf(fp, "%f %f\n", &uv.x, &uv.y);
					tu.push_back(uv);
				}

				if (strcmp(line, "vn") == 0)
				{
					C_Vector3 norm;
					fscanf(fp, "%f %f %f\n", &norm.x, &norm.y, &norm.z);
					tn.push_back(norm);
				}

				if (strcmp(line, "f") == 0)
				{
					if (tv.size() != 0 && tu.size() != 0 && tn.size() != 0)
					{
						int v1, v2, v3, t1, t2, t3, n1, n2, n3;
						fscanf(fp, "%i/%i/%i %i/%i/%i %i/%i/%i\n", &v1, &t1, &n1, &v2, &t2, &n2, &v3, &t3, &n3);

						C_Vertex ver1, ver2, ver3;

						ver1.pos = tv[v1 - 1];
						ver2.pos = tv[v2 - 1];
						ver3.pos = tv[v3 - 1];

						ver1.UV = tu[t1 - 1];
						ver2.UV = tu[t2 - 1];
						ver3.UV = tu[t3 - 1];

						ver1.normal = tn[n1 - 1];
						ver2.normal = tn[n2 - 1];
						ver3.normal = tn[n3 - 1];

						verts.push_back(ver1);
						verts.push_back(ver2);
						verts.push_back(ver3);
					}
				}
			}

			tv.clear();
			tu.clear();
			tn.clear();

			return verts;
		}

}

}
