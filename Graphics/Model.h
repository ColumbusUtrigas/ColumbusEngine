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
	private:
		C_Vector3 mPos;
		C_Mesh* mParent = nullptr;
		C_Camera mCamera;

		std::list<C_Mesh*> mChilds;
		std::vector<C_Light*> mLights;

		C_Buffer* mVBuf = nullptr; //Vertex buffer
		C_Buffer* mUBuf = nullptr; //UV buffer
		C_Buffer* mNBuf = nullptr; //Normal buffer
		C_Buffer* mTBuf = nullptr; //Tangent buffer
		C_Buffer* mBBuf = nullptr; //Bitangent buffer

		float mLightUniform[120];

		//Sets texture\cubemap as uniform in shader
		void setShaderTextures();
		//Calculate and transfer matrices as uniform in shader
		void setShaderMatrices(C_Transform aTransform);
		//Set all material data as uniform in shader
		void setShaderMaterial();
		//Set all lights and camera data as unniform in shader
		void setShaderLightAndCamera();
		//Calculate lights
		void calculateLights();
		//Sort lights array by distance
		void sortLights();
	public:
		std::vector<C_Vertex> mVert;
		C_Material mMat;
		
		C_Mesh();
		C_Mesh(std::string aFile);
		C_Mesh(std::vector<C_Vertex> aVert);
		C_Mesh(std::vector<C_Vertex> aVert, C_Material aMat);

		void setVertices(std::vector<C_Vertex> aVert);
		void render(C_Transform aTransform);
		
		void setPos(C_Vector3 aPos);
		void addPos(C_Vector3 aPos);
		C_Vector3 getPos() const;

		void setCamera(C_Camera camera);
		void setParent(C_Mesh* aParent);
		void addChild(C_Mesh* aChild);
		void setLights(std::vector<C_Light*> aLights);

		void clear();

		~C_Mesh();
	};

}
