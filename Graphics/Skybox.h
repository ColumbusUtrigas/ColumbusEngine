#pragma once

#include <Graphics/Texture.h>
#include <Graphics/Camera.h>
#include <Graphics/Shader.h>

namespace Columbus
{

	class Skybox
	{
	private:
		uint32 VBO = 0;
		uint32 VAO = 0;
		ShaderProgram* Shader = nullptr;
		Texture* Tex = nullptr;
		Texture* IrradianceMap = nullptr;

		Camera ViewCamera;
	public:
		Skybox();
		Skybox(Texture* InTexture);
		
		void Render();

		void SetCamera(const Camera& Cam);
		Texture* GetCubemap() const { return Tex; }
		Texture* GetIrradianceMap() const { return IrradianceMap; }
		
		~Skybox();
	};

}
