#pragma once

#include <Graphics/Texture.h>
#include <Graphics/Camera.h>
#include <Graphics/Shader.h>
#include <Graphics/Buffer.h>

namespace Columbus
{

	class Skybox
	{
	private:
		uint32 VBO = 0;
		uint32 IBO = 0;
		uint32 VAO = 0;
		ShaderProgram* Shader = nullptr;

		Texture* Tex = nullptr;
		Texture* IrradianceMap = nullptr;
		Texture* PrefilterMap = nullptr;
		Texture* IntegrationMap = nullptr;

		Camera ViewCamera;
	public:
		Skybox();
		Skybox(Texture* InTexture);

		void Render();

		void SetCamera(const Camera& Cam);
		Texture* GetCubemap() const { return Tex; }
		Texture* GetIrradianceMap() const { return IrradianceMap; }
		Texture* GetPrefilterMap() const { return PrefilterMap; }
		Texture* GetIntegrationMap() const { return IntegrationMap; }

		~Skybox();
	};

}


