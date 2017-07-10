#pragma once

#include <Cubemap.h>
#include <Camera.h>
#include <Buffer.h>
#include <Shader.h>

namespace C
{

	class C_Skybox
	{
	private:
		C_Buffer* mBuf = NULL;
		C_Shader* mShader = NULL;
	public:
		C_Cubemap* mCubemap = NULL;

		C_Skybox() {}
		C_Skybox(C_Cubemap* aCubemap);

		void draw();

		~C_Skybox();
	};

}



