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
		Camera mCamera;
	public:
		Skybox();
		Skybox(Texture* InTexture);
		
		void draw();

		void setCamera(const Camera aCamera);
		Texture* GetCubemap() const { return Tex; }
		
		~Skybox();
	};

}
