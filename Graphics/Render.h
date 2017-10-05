/************************************************
*                  Render.h                     *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*             Nikolay(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#pragma once

#include <Graphics/Model.h>
#include <Graphics/Camera.h>
#include <Graphics/Skybox.h>
#include <Graphics/ParticleEmitter.h>
#include <Graphics/Framebuffer.h>
#include <Graphics/Renderbuffer.h>

namespace C
{

	class C_Render
	{
	private:
		std::vector<C_Mesh*> mMeshes;
		std::vector<C_ParticleEmitter*> mParticleEmitters;
		C_Camera* mCamera = nullptr;
		C_Skybox* mSkybox = nullptr;

		C_Shader* mPostProcess = NULL;

		unsigned int FBO = 0;
		unsigned int TBO = 0;
		unsigned int RBO = 0;

		C_Framebuffer* FB = nullptr;
		C_Texture* TB = nullptr;
		C_Renderbuffer* RB = nullptr;

		C_Vector2 mWindowSize;

		//Draw screen quad
		void drawQuad();
	public:
		//Constructor
		C_Render();
		//Add mesh
		void add(C_Mesh* aMesh);
		//Add particle emmitter
		void add(C_ParticleEmitter* aP);
		//Set main camera
		void setMainCamera(C_Camera* aCamera);
		//Set window size
		void setWindowSize(C_Vector2 aWindowSize);
		//Set skybox
		void setSkybox(C_Skybox* aSkybox);
		//Render scene
		void render();
		//Destructor
		~C_Render();
	};

}
