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
#include <Graphics/Light.h>
#include <Graphics/ParticleEmitter.h>
#include <Graphics/Framebuffer.h>
#include <Graphics/Renderbuffer.h>
#include <Graphics/PostEffect.h>
#include <RenderAPI/APIOpenGL.h>
#include <System/Timer.h>

#include <GUI/Button.h>

namespace C
{

	class C_Render
	{
	private:
		std::vector<C_Mesh*> mMeshes;
		std::vector<C_ParticleEmitter*> mParticleEmitters;
		std::vector<C_Light*> mLights;
		C_Camera* mCamera = nullptr;
		C_Skybox* mSkybox = nullptr;

		C_Timer mFrameTimer;

		C_Shader* mNonePost = nullptr;
		C_Shader* mNegativePost = nullptr;
		C_Shader* mGaussianPost = nullptr;

		C_Vector2 mWindowSize;

		C_PostEffect mNone;
		C_PostEffect mNegative;
		C_PostEffect mGaussianBlur;

		//Enable all OpenGL varyables
		void enableAll();
		//Prepare scene to rendering
		void prepareScene();
		//Render scene
		void renderScene();
	public:
		//Constructor
		C_Render();
		//Add mesh
		void add(C_Mesh* aMesh);
		//Add particle emmitter
		void add(C_ParticleEmitter* aP);
		//Add light
		void add(C_Light* aLight);
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
