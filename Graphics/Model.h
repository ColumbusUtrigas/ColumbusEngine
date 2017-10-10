/************************************************
*                   Model.h                     *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*             Nikolay(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#pragma once

#include <vector>
#include <string>
#include <list>

#include <RenderAPI/Buffer.h>

#include <Graphics/Camera.h>
#include <Graphics/Shader.h>
#include <Graphics/Texture.h>
#include <Graphics/Material.h>
#include <Graphics/Importer.h>
#include <Graphics/Skybox.h>

namespace C
{

	class C_Mesh
	{
	private:
		C_Vector3 mPos;
		C_Vector3 mRot;
		C_Vector3 mScale;
		C_Vector3 mPivot;
		glm::mat4 mMatrix;

		C_Mesh* mParent = NULL;

		C_Camera mCamera;

		std::list<C_Mesh*> mChilds;
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
