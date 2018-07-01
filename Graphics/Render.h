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
#include <Graphics/MeshInstanced.h>
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


		~C_Render();
	};

	class Renderer
	{
	protected:
		struct MeshRenderData
		{
			Mesh* Object;
			Transform ObjectTransform;
			Material ObjectMaterial;

			MeshRenderData(Mesh* InObject, Transform InTransform, Material InMaterial) :
				Object(InObject),
				ObjectTransform(InTransform),
				ObjectMaterial(InMaterial) {}
		};
	protected:
		std::map<uint32, SmartPointer<GameObject>>* RenderList;
		
		std::vector<MeshRenderData> Meshes;
		std::vector<MeshInstanced*> MeshesInstanced;
		std::vector<ParticleEmitter*> ParticleEmitters;

		Camera MainCamera;
	public:
		enum class Stage
		{
			Opaque,
			Transparent
		};
	public:
		Renderer();

		void SetMainCamera(Camera& InCamera) { MainCamera = InCamera; }

		virtual void SetRenderList(std::map<uint32, SmartPointer<GameObject>>* List);
		virtual void CompileLists();
		virtual void Render(Stage RenderStage);
		virtual void Render(std::map<uint32, SmartPointer<GameObject>>* RenderList);

		~Renderer();
	};

}
