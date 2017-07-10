#pragma once

#include <Camera.h>
#include <Shader.h>
#include <Texture.h>
#include <Buffer.h>
#include <Material.h>
#include <Importer.h>
#include <Skybox.h>
#include <vector>

namespace C
{

	class C_Mesh
	{
	private:
		C_Vector3 mPos;
		C_Vector3 mRot;
		C_Vector3 mScale;
		glm::mat4 mMatrix;

		C_Mesh* mParent = NULL;

		C_Camera mCamera;

		std::vector<C_Mesh*> mChilds;
	public:
		C_Buffer* buf = NULL;
		C_Buffer* tbuf = NULL;
		C_Buffer* nbuf = NULL;

		std::vector<C_Vertex> mVert;
		C_Material mMat;

		C_Mesh();
		C_Mesh(std::vector<C_Vertex> aVert);

		void draw();

		void setPos(C_Vector3 aPos);

		void setRot(C_Vector3 aRot);

		void setScale(C_Vector3 aScale);

		void addPos(C_Vector3 aPos);

		void addRot(C_Vector3 aRot);

		void addScale(C_Vector3 aScale);

		void setCamera(C_Camera camera);

		bool load(const char* aFile);

		void clear();

		~C_Mesh();
	};

}





