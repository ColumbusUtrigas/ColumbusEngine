/************************************************
*                    Model.h                    *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   08.01.2018                  *
*************************************************/
#pragma once

#include <System/Assert.h>
#include <System/System.h>
#include <Math/Vector2.h>
#include <Math/Vector3.h>
#include <Math/Vector4.h>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <cstring>

namespace Columbus
{

	struct Vertex
	{
		Vector3 pos;
		Vector2 UV;
		Vector3 normal;
		Vector3 tangent;
		Vector3 bitangent;
	};

	bool ModelIsCMF(const std::string aFile);

	std::vector<Vertex> ModelLoadCMF(const std::string aFile);

	class C_Model
	{
	private:
		std::vector<Vertex> mVertices;
		std::string mFilename;
		bool mExistance = false;
	public:
		C_Model();
		C_Model(const std::string aFile);

		bool load(const std::string aFile); //Load model from OBJ or CMF format
		bool save(const std::string aFile) const; //Save to CMF format ONLY!!!
		bool isExist() const;
		bool freeData();

		std::vector<Vertex> getData() const;
		std::string getFilename() const;

		~C_Model();
	};

}




