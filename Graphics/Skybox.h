#pragma once

#include <Graphics/Texture.h>
#include <Graphics/Camera.h>

namespace Columbus
{

	class Skybox
	{
	private:
		uint32 VBO = 0;
		uint32 IBO = 0;
		uint32 VAO = 0;

		Texture* Tex = nullptr;
		Texture* IrradianceMap = nullptr;
		Texture* PrefilterMap = nullptr;

		Camera ViewCamera;
	public:
		Skybox();
		Skybox(Texture* InTexture);

		void Render();

		void SetCamera(const Camera& Cam);
		Texture* GetCubemap() const { return Tex; }
		Texture* GetIrradianceMap() const { return IrradianceMap; }
		Texture* GetPrefilterMap() const { return PrefilterMap; }

		~Skybox();
	};

}


