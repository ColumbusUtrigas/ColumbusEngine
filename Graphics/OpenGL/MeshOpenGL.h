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

		uint64 VOffset = 0;
		uint64 UOffset = 0;
		uint64 NOffset = 0;
		uint64 TOffset = 0;

		float mLightUniform[120];
		float mMaterialUnif[15];
		float UniformModelMatrix[16];
		float UniformViewMatrix[16];
		float UniformProjectionMatrix[16];

		void setShaderTextures();
		void setShaderMatrices(Transform InTransform);
		void setShaderMaterial();
		void setShaderLightAndCamera();
		void calculateLights();
		void sortLights();
	public:
		MeshOpenGL();
		MeshOpenGL(std::vector<Vertex> aVert);
		MeshOpenGL(std::vector<Vertex> aVert, Material aMaterial);

		void setVertices(std::vector<Vertex> aVert) override;
		void render(Transform InTransform) override;

		~MeshOpenGL() override;
	};

}













