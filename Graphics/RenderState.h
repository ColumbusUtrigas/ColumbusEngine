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
	public:
		Texture* ShadowTexture = nullptr;
		Texture* TranslucentTex = nullptr;
		iVector2 ContextSize{ 1 };
	public:
		void SetCulling(Material::Cull Culling);
		void SetBlending(bool Blending);
		void SetDepthTesting(Material::DepthTest DepthTesting);
		void SetDepthWriting(bool DepthWriting);
		void SetMainCamera(const Camera& InMainCamera);
		void SetMaterial(const Material& InMaterial, const Matrix& ModelMatrix, Skybox* Sky, bool UsePixelShader = true);
	};

}


