/************************************************
*                  Render.h                     *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
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
#include <Scene/GameObject.h>

namespace Columbus
{

	class C_Render
	{
	private:
		std::vector<C_Mesh*> mMeshes;
		std::vector<C_ParticleEmitter*> mParticleEmitters;

		C_Skybox* mSkybox = nullptr;

		C_Shader* mMeshPrepass = nullptr;
		C_Shader* mParticlesPrepass = nullptr;

		//Enable all OpenGL varyables
		void enableAll();
		//Prepass scene beffore rendering
		void prepassScene();
		//Render scene
		void renderScene();
	public:
		//Constructor
		C_Render();
		//Add mesh
		void add(C_Mesh* aMesh);
		//Add particle emmitter
		void add(C_ParticleEmitter* aP);
		//Set skybox
		void setSkybox(C_Skybox* aSkybox);
		//Render scene
		void render();

		static void render(C_GameObject* aObject);

		//Destructor
		~C_Render();
	};

}
