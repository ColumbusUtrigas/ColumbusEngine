#pragma once

#include <Graphics/Material.h>
#include <Graphics/Camera.h>
#include <Graphics/Shader.h>
#include <Graphics/Skybox.h>
#include <Graphics/Mesh.h>
#include <vector>

namespace Columbus
{

	class RenderState
	{
	private:
		Camera MainCamera;

		Material PreviousMaterial;
		Material  CurrentMaterial;

		ShaderProgram* PreviousShader;
		ShaderProgram*  CurrentShader;

		Mesh* PreviousMesh;
		Mesh*  CurrentMesh;

		Material::Cull PreviousCulling;
		bool PreviousBlending;
		Material::DepthTest PreviousDepthTesting;
		bool PreviousDepthWriting;
	public:
		Texture* ShadowTexture = nullptr;
	public:
		RenderState() { /*Clear();*/ }

		void Clear();

		void SetCulling(Material::Cull Culling);
		void SetBlending(bool Blending);
		void SetDepthTesting(Material::DepthTest DepthTesting);
		void SetDepthWriting(bool DepthWriting);
		void SetMainCamera(const Camera& InMainCamera);
		void SetMaterial(const Material& InMaterial, const Matrix& ModelMatrix, Skybox* Sky);
		void SetLights(const std::vector<Light*>& InLights, const int32 LightIndices[4]);
		void SetShaderProgram(ShaderProgram* InShaderProgram);
		void SetMesh(Mesh* InMesh);
	};

}


