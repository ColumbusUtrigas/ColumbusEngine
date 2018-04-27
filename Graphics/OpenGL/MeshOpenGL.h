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
		uint32 VBuf = 0;

		uint64 VOffset = 0;
		uint64 UOffset = 0;
		uint64 NOffset = 0;
		uint64 TOffset = 0;

		float mLightUniform[120];
		float mMaterialUnif[15];
		float UniformModelMatrix[16];
		float UniformViewMatrix[16];
		float UniformProjectionMatrix[16];

		void SetShaderTextures();
		void SetShaderMatrices(Transform InTransform);
		void SetShaderMaterial();
		void SetShaderLightAndCamera();
		void CalculateLights();
		void SortLights();
	public:
		MeshOpenGL();
		MeshOpenGL(std::vector<Vertex> aVert);
		MeshOpenGL(std::vector<Vertex> aVert, Material aMaterial);

		void SetVertices(std::vector<Vertex> aVert) override;
		void Render(Transform InTransform) override;

		~MeshOpenGL() override;
	};

}













