/************************************************
*                 Importer.h                    *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*             Nikolay(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include <Math/Vector2.h>
#include <Math/Vector3.h>
#include <Math/Vector4.h>

#include <RenderAPI/Buffer.h>

namespace C
{

	struct C_Vertex
	{
		C_Vector3 pos;
		C_Vector2 UV;
		C_Vector3 normal;
		C_Vector3 tangent;
		C_Vector3 bitangent;
	};

	namespace Importer
	{

		class C_Importer
		{
		private:
			////////////////////////////////////////////////////////////////////////////
			//Load OBJ file
			inline bool loadOBJ(std::string aFile)
			{
				if (aFile.empty())
					return false;

				std::ifstream file;
				file.open(aFile);

				std::string line;

				std::vector<C_Vector3> tv;
				std::vector<C_Vector2> tu;
				std::vector<C_Vector3> tn;
				std::vector<C_Vertex> vert;

				while (std::getline(file, line))
				{
					std::istringstream iss(line);

					if (line.substr(0, 1) == "#")
						continue;

					if (line.substr(0, 2) == "v ")
					{
						float x, y, z;
						char c;
						iss >> c;
						iss >> x >> y >> z;
						tv.push_back(C_Vector3(x, y, z));
					}
					else if (line.substr(0, 3) == "vt ")
					{
						float x, y;
						char c;
						iss >> c >> c;
						iss >> x >> y;
						tu.push_back(C_Vector2(x, y));
					}
					else if (line.substr(0, 3) == "vn ")
					{
						float x, y, z;
						char c;
						iss >> c >> c;
						iss >> x >> y >> z;
						tn.push_back(C_Vector3(x, y, z));
					}
					else if (line.substr(0, 2) == "o ")
					{
						/*if (tv.size() > 0)
							tv.clear();
						if (tu.size() > 0)
							tu.clear();
						if (tn.size() > 0)
							tn.clear();*/

						if (vert.size () > 0)
						{
							mVertices.push_back(vert);
							vert.clear();
						}

						char c;
						std::string name;
						iss >> c >> name;
						mNames.push_back(name);
						mObjsCount++;
					} else if (line.substr(0, 2) == "f ")
					{
						int v1, v2, v3, t1, t2, t3, n1, n2, n3;
						char c;

						iss >> c;

						iss >> v1 >> c >> t1 >> c >> n1;
						iss >> v2 >> c >> t2 >> c >> n2;
						iss >> v3 >> c >> t3 >> c >> n3;

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

						C_Vector3 deltaPos1 = ver2.pos - ver1.pos;
						C_Vector3 deltaPos2 = ver3.pos - ver1.pos;

						C_Vector2 deltaUV1 = ver2.UV - ver1.UV;
						C_Vector2 deltaUV2 = ver3.UV - ver1.UV;

						float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
						C_Vector3 tangent = (deltaPos1 * deltaUV2.y   - deltaPos2 * deltaUV1.y) * r;
						C_Vector3 bitangent = (deltaPos2 * deltaUV1.x   - deltaPos1 * deltaUV2.x) * r;

						ver1.tangent = tangent;
						ver2.tangent = tangent;
						ver3.tangent = tangent;

						ver1.bitangent = bitangent;
						ver2.bitangent = bitangent;
						ver3.bitangent = bitangent;

						vert.push_back(ver1);
						vert.push_back(ver2);
						vert.push_back(ver3);
					}
				}

				file.close();

				if (tv.size() > 0)
					tv.clear();
				if (tu.size() > 0)
					tu.clear();
				if (tn.size() > 0)
					tn.clear();

				if (vert.size () > 0)
				{
					mVertices.push_back(vert);
					vert.clear();
				}

				return true;
			}
		public:
			int mObjsCount = 0;
			std::vector<std::string> mNames;
			std::vector<std::vector<C_Vertex>> mVertices;

			////////////////////////////////////////////////////////////////////////////
			//Constructor
			inline C_Importer() {}
			////////////////////////////////////////////////////////////////////////////
			//Load file
			inline bool load(std::string aFile)
			{
				if (aFile.empty())
					return false;

				if (aFile.substr(aFile.size() - 4) == ".obj")
					return loadOBJ(aFile);
			}
			////////////////////////////////////////////////////////////////////////////
			//Destructor
			inline ~C_Importer() {}
		};
		////////////////////////////////////////////////////////////////////////////
		//Load vertices from OBJ file
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
