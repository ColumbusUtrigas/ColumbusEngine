#pragma once

#include <Graphics/Texture.h>
#include <Graphics/Camera.h>
#include <Graphics/Device.h>

namespace Columbus
{

	class Skybox
	{
	private:
		SPtr<Buffer> cbuffer;
		SPtr<Buffer> vbuffer;
		SPtr<Buffer> ibuffer;

		SPtr<Texture> tex;

		Texture* Tex = nullptr;
		Texture* IrradianceMap = nullptr;
		Texture* PrefilterMap = nullptr;

		Camera ViewCamera;
	public:
		Skybox();
		Skybox(SPtr<Texture> hdri);
		Skybox(Texture* InTexture);

		void Render();

		void SetCamera(const Camera& Cam);
		Texture* GetCubemap() const { return Tex; }
		Texture* GetIrradianceMap() const { return IrradianceMap; }
		Texture* GetPrefilterMap() const { return PrefilterMap; }

		~Skybox();
	};

}
