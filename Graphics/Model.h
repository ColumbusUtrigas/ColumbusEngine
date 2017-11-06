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

namespace Columbus
{

	class C_Mesh
	{
	private:
		C_Vector3 mPos;
		C_Vector3 mRot;
		C_Vector3 mScale;
		C_Vector3 mPivot;
		glm::mat4 mMatrix;
		glm::mat4 mNormalMatrix;

		C_Mesh* mParent = NULL;
		
		C_Camera mCamera;

		std::list<C_Mesh*> mChilds;
		std::vector<C_Light*> mLights;

		float mLightUniform[120];

		//Sets texture\cubemap as uniform in shader
		void setShaderTextures();
		//Calculate and transfer matrices as uniform in shader
		void setShaderMatrices();
		//Set all material data as uniform in shader
		void setShaderMaterial();
		//Set all lights and camera data as unniform in shader
		void setShaderLightAndCamera();
		//Calculate lights
		void calculateLights();
		//Sort lights array by distance
		void sortLights();
	public:
		C_Buffer* buf = NULL;
		C_Buffer* tbuf = NULL;
		C_Buffer* nbuf = NULL;
		C_Buffer* tangbuf = NULL;
		C_Buffer* bitangbuf = NULL;

		std::vector<C_Vertex> mVert;
		C_Material mMat;
		//Constructor
		C_Mesh();
		//Constructor 2
		C_Mesh(std::string aFile);
		//Constructor 3
		C_Mesh(std::vector<C_Vertex> aVert);
		//Draw mesh
		void draw();
		//Set mesh position
		void setPos(C_Vector3 aPos);
		//Set mesh rotation
		void setRot(C_Vector3 aRot);
		//Set mesh scale
		void setScale(C_Vector3 aScale);
		//Add position to current
		void addPos(C_Vector3 aPos);
		//Add rotation to current
		void addRot(C_Vector3 aRot);
		//Add scale to current
		void addScale(C_Vector3 aScale);
		//Set camera
		void setCamera(C_Camera camera);
		//Set parent mesh
		void setParent(C_Mesh* aParent);
		//Add child mesh
		void addChild(C_Mesh* aChild);
		//Set pivot point
		void setPivot(C_Vector3 aPivot);
		//Set light casters, which calculate to using in shaders
		void setLights(std::vector<C_Light*> aLights);
		//Return pivot point
		C_Vector3 getPivot();
		//Add position to pivot point
		void addPivot(C_Vector3 aPivot);
		//Clear all
		void clear();
		//Destructor
		~C_Mesh();
	};

}
