#pragma once

#include <Camera.h>
#include <Shader.h>
#include <Texture.h>
#include <Buffer.h>
#include <Material.h>
#include <vector>

namespace C
{

	struct C_Vertex
	{
		C_Vector3 pos;
		C_Vector2 UV;
		C_Vector3 normal;
	};

	class C_Mesh
	{
	private:
		C_Vector3 mPos;
		C_Vector3 mRot;
		C_Vector3 mScale;
		glm::mat4 mMatrix;

		C_Mesh* mParent;

	public:
		C_Buffer* buf = NULL;
		C_Buffer* tbuf = NULL;
		C_Buffer* nbuf = NULL;

		std::vector<C_Vertex> mVert;
		C_Material mMat;

		C_Mesh();
		C_Mesh(std::vector<C_Vertex> aVert);

		void draw(C_Shader aShader);

		void loadOBJ(const char* aFile);

		~C_Mesh();
	};

}





