/************************************************
*                 Importer.cpp                  *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   02.11.2017                  *
*************************************************/

#include <System/Importer.h>

namespace Columbus
{

	namespace Import
	{

		C_ImporterModel::C_ImporterModel() :
			mObjectsCount(0)
		{

		}
		////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////
		bool load(const std::string aFile)
		{

		}
		////////////////////////////////////////////////////////////////////////////
		bool C_ImporterModel::loadOBJ(const std::string aFile)
		{
			if (aFile.empty()) return false;

			std::ifstream file;
			file.open(aFile);

			if (!file.is_open()) return false;

			std::string line;

			std::vector<C_Vector3> tv;
			std::vector<C_Vector2> tu;
			std::vector<C_Vector3> tn;
			std::vector<C_Vertex> vert;

			while (std::getline(file, line))
			{
				std::istringstream iss(line);

				if (line.substr(0, 1) == "#") continue;

				if (line.substr(0, 2) == "v ")
				{
					float x, y, z;
					char c;
					iss >> c >> x >> y >> z;
					tv.push_back(C_Vector3(x, y, z));
				}
				else if (line.substr(0, 3) == "vt ")
				{
					float x, y;
					char c;
					iss >> c >> c >> x >> y;
					tu.push_back(C_Vector2(x, y));
				}
				else if (line.substr(0, 3) == "vn ")
				{
					float x, y, z;
					char c;
					iss >> c >> c >> x >> y >> z;
					tn.push_back(C_Vector3(x, y, z));
				}
				else if (line.substr(0, 2) == "o ")
				{
					if (vert.size() > 0)
					{
						mVertices.push_back(vert);
						vert.clear();
					}

					char c;
					std::string name;
					iss >> c >> name;
					mNames.push_back(name);
					mObjectsCount++;
				}
				else if (line.substr(0, 2) == "f ")
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
					C_Vector3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
					C_Vector3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;

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
				tv.erase(tv.begin(), tv.end());
			if (tu.size() > 0)
				tu.erase(tu.begin(), tu.end());
			if (tn.size() > 0)
				tn.erase(tn.begin(), tn.end());

			if (vert.size() > 0)
			{
				mVertices.push_back(vert);
				vert.clear();
			}

			return true;
		}
		////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////
		unsigned int C_ImporterModel::getCount() const
		{
			return mObjectsCount;
		}
		////////////////////////////////////////////////////////////////////////////
		std::vector<std::string> C_ImporterModel::getNames() const
		{
			return mNames;
		}
		////////////////////////////////////////////////////////////////////////////
		std::vector<std::vector<C_Vertex>> C_ImporterModel::getObjects() const
		{
			return mVertices;
		}
		////////////////////////////////////////////////////////////////////////////
		std::string C_ImporterModel::getName(unsigned int aID) const
		{
			return mNames.at(aID);
		}
		////////////////////////////////////////////////////////////////////////////
		std::vector<C_Vertex> C_ImporterModel::getObject(unsigned int aID) const
		{
			return mVertices.at(aID);
		}
		////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////
		C_ImporterModel::~C_ImporterModel()
		{

		}

	}

}






