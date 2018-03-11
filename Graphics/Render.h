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

#include <Graphics/Mesh.h>
#include <Graphics/Camera.h>
#include <Graphics/Skybox.h>
#include <Graphics/Light.h>
#include <Graphics/Particles/ParticleEmitter.h>
#include <Graphics/Framebuffer.h>
#include <Graphics/Renderbuffer.h>
#include <Graphics/PostEffect.h>
#include <RenderAPI/APIOpenGL.h>
#include <System/Timer.h>
#include <Scene/GameObject.h>
#include <Scene/MeshRenderer.h>
#include <Scene/ParticleSystem.h>
#include <Scene/Component.h>

namespace Columbus
{

	class C_Render
	{
	private:
		//Enable all OpenGL varyables
		void enableAll();
	public:
		//Constructor
		C_Render();
		static void enableDepthPrepass();
		static void renderDepthPrepass(GameObject* aGameObject);
		static void disableDepthPrepass();
		static void render(GameObject* aGameObject);
		//Destructor
		~C_Render();
	};

}
