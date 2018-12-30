#pragma once

#include <Graphics/Mesh.h>
#include <Graphics/Camera.h>
#include <Graphics/Skybox.h>
#include <Graphics/Light.h>
#include <Graphics/Particles/ParticleEmitter.h>
#include <Graphics/Framebuffer.h>
#include <Graphics/PostEffect.h>
#include <System/Timer.h>
#include <Scene/GameObject.h>

#include <Graphics/ScreenQuad.h>

#include <map>

namespace Columbus
{

	class Renderer
	{
	protected:
		struct OpaqueRenderData
		{
			Mesh* Object;
			Transform ObjectTransform;
			Material ObjectMaterial;

			OpaqueRenderData(Mesh* InObject, const Transform& InTransform, const Material& InMaterial) :
				Object(InObject),
				ObjectTransform(InTransform),
				ObjectMaterial(InMaterial) {}
		};

		struct TransparentRenderData
		{
			Mesh* MeshObject;
			ParticleEmitter* ParticleObject;

			Transform ObjectTransform;
			Material ObjectMaterial;

			TransparentRenderData(Mesh* InMesh, ParticleEmitter* InParticles, const Transform& InTransform, const Material& InMaterial) :
				MeshObject(InMesh),
				ParticleObject(InParticles),
				ObjectTransform(InTransform),
				ObjectMaterial(InMaterial) {}
		};
	protected:
		std::map<uint32, SmartPointer<GameObject>>* RenderList;

		std::vector<OpaqueRenderData> OpaqueObjects;
		std::vector<TransparentRenderData> TransparentObjects;
		
		iVector2 ContextSize;
		Camera MainCamera;
		Skybox* Sky = nullptr;

		PostEffect BaseEffect;
		PostEffect BloomBrightPass;
		PostEffect BloomBlurPass;
		PostEffect BloomFinalPass;

		ScreenQuad Quad;
	public:
		enum class Stage
		{
			Opaque,
			Transparent
		};
	public:
		Renderer();

		void SetContextSize(const iVector2& Size) { ContextSize = Size; }
		void SetMainCamera(const Camera& InCamera) { MainCamera = InCamera; }
		void SetSky(Skybox* InSky) { Sky = InSky; }

		virtual void SetRenderList(std::map<uint32, SmartPointer<GameObject>>* List);
		virtual void CompileLists();
		virtual void SortLists();

		virtual void RenderOpaqueStage();
		virtual void RenderTransparentStage();
		virtual void Render(Stage RenderStage);
		virtual void Render();

		~Renderer();
	};

}
