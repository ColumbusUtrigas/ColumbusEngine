/************************************************
*                 Importer.h                    *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*             Nikolay(Columbus) Red             *
*                   02.11.2017                  *
*************************************************/

#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

#include <Math/Vector2.h>
#include <Math/Vector3.h>
#include <Math/Vector4.h>
#include <System/Console.h>

namespace Columbus
{

	struct C_Vertex
	{
		C_Vector3 pos;
		C_Vector2 UV;
		C_Vector3 normal;
		C_Vector3 tangent;
		C_Vector3 bitangent;
	};

	namespace Import
	{

		class C_ImporterModel
		{
		private:
			unsigned int mObjectsCount = 0;
			std::vector<std::string> mNames;
			std::vector<std::vector<C_Vertex>> mVertices;
		public:
			C_ImporterModel();

			bool loadOBJ(std::string aFile);

			unsigned int getCount() const;
			std::vector<std::string> getNames() const;
			std::vector<std::vector<C_Vertex>> getObjects() const;
			std::string getName(unsigned int aID) const;
			std::vector<C_Vertex> getObject(unsigned int aID) const;

			~C_ImporterModel();
		};

	}

}

