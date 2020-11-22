#pragma once

#include <Graphics/Texture.h>
#include <Graphics/Camera.h>
#include <Graphics/Device.h>
#include <memory>

namespace Columbus
{

	class Skybox
	{
	private:
		BlendState* BS;
		DepthStencilState* DSS;
		RasterizerState* RS;

		Buffer* VertexBuffer;
		Buffer* IndexBuffer;
		std::shared_ptr<InputLayout> Layout;

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
