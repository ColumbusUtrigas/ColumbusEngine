/************************************************
*                   MeshOpenGL.h                *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   16.01.2018                  *
*************************************************/
#pragma once

#include <Graphics/Mesh.h>

namespace Columbus
{

	class MeshOpenGL : public Mesh
	{
	private:
		unsigned int mVBuf = 0;

		size_t mVOffset = 0;
		size_t mUOffset = 0;
		size_t mNOffset = 0;
		size_t mTOffset = 0;

		float mLightUniform[120];
		float mMaterialUnif[14];

		void setShaderTextures();
		void setShaderMatrices(Transform aTransform);
		void setShaderMaterial();
		void setShaderLightAndCamera();
		void calculateLights();
		void sortLights();
	public:
		MeshOpenGL();
		MeshOpenGL(std::vector<Vertex> aVert);
		MeshOpenGL(std::vector<Vertex> aVert, C_Material aMaterial);

		void setVertices(std::vector<Vertex> aVert) override;
		void render(Transform aTransform) override;

		~MeshOpenGL();
	};

}













