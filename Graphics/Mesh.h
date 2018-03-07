/************************************************
*                   Model.h                     *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#pragma once

#include <vector>
#include <string>
#include <list>
#include <algorithm>

#include <System/Importer.h>
#include <RenderAPI/Buffer.h>

#include <Graphics/Camera.h>
#include <Graphics/Shader.h>
#include <Graphics/Texture.h>
#include <Graphics/Material.h>
#include <Graphics/Skybox.h>
#include <Graphics/Light.h>

#include <Scene/Transform.h>

namespace Columbus
{

	class C_Mesh
	{
	protected:
		C_Vector3 mPos;
		C_Mesh* mParent = nullptr;
		C_Camera mCamera;

		std::list<C_Mesh*> mChilds;
		std::vector<C_Light*> mLights;
	public:
		std::vector<C_Vertex> mVert;
		C_Material mMat;
		
		C_Mesh();
		C_Mesh(std::vector<C_Vertex> aVert);
		C_Mesh(std::vector<C_Vertex> aVert, C_Material aMat);

		virtual void setVertices(std::vector<C_Vertex> aVert);
		virtual void render(C_Transform aTransform);

		void setCamera(C_Camera camera);
		void setParent(C_Mesh* aParent);
		void addChild(C_Mesh* aChild);
		void setLights(std::vector<C_Light*> aLights);

		void clear();

		~C_Mesh();
	};

}
