/************************************************
*                 Importer.h                    *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   02.11.2017                  *
*************************************************/

#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

#include <Common/Model/Model.h>
#include <System/Log.h>

namespace Columbus
{

	namespace Import
	{

		class ImporterModel
		{
		private:
			unsigned int mObjectsCount = 0;
			std::vector<std::string> mNames;
			std::vector<std::vector<Vertex>> mVertices;
		public:
			ImporterModel();

			bool load(const std::string aFile);
			bool loadOBJ(const std::string aFile);

			unsigned int getCount() const;
			std::vector<std::string> getNames() const;
			std::vector<std::vector<Vertex>> getObjects() const;
			std::string getName(unsigned int aID) const;
			std::vector<Vertex> getObject(unsigned int aID) const;

			~ImporterModel();
		};

	}

}

