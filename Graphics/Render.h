#pragma once

#include <Graphics/Mesh.h>
#include <Graphics/Camera.h>
#include <Graphics/Skybox.h>
#include <Graphics/Light.h>
#include <Graphics/Particles/ParticleEmitterCPU.h>
#include <Graphics/Framebuffer.h>
#include <Graphics/PostEffect.h>
#include <Scene/GameObject.h>
#include <Scene/Scene.h>

#include <Graphics/ScreenQuad.h>
#include <Graphics/ParticlesRenderer.h>

#include <map>

namespace Columbus
{

	class Renderer
	{
	protected:
		struct OpaqueRenderData
		{
			Mesh* Object;
			uint32 Index; // Index of GameObject in array
			int32 Lights[4] = { -1, -1, -1, -1 };

			OpaqueRenderData(Mesh* InObject, uint32 InIndex) :
				Object(InObject),
				Index(InIndex) {}
		};

		struct TransparentRenderData
		{
			Mesh* MeshObject;
			ParticleEmitterCPU* Particles;
			uint32 Index; // Index of GameObject in array
			int32 Lights[4] = { -1, -1, -1, -1 };

			TransparentRenderData(Mesh* InMesh, uint32 InIndex) :
				MeshObject(InMesh),
				Particles(nullptr),
				Index(InIndex) {}

			TransparentRenderData(ParticleEmitterCPU* CPU, uint32 InIndex) :
				MeshObject(nullptr),
				Particles(CPU),
				Index(InIndex) {}
		};
	protected:
		std::vector<SmartPointer<GameObject>>* RenderList;
		std::vector<Light*>* LightsList;
		std::vector<std::pair<uint32, Light*>> LightsPairs;

		std::vector<OpaqueRenderData> OpaqueObjects;
		std::vector<TransparentRenderData> TransparentObjects;
		
		iVector2 ViewportOrigin;
		iVector2 ViewportSize;

		Camera MainCamera;
		Skybox* Sky = nullptr;
		Scene* Scn = nullptr;

		PostEffect BaseEffect;
		PostEffect BloomBrightPass;
		PostEffect BloomHorizontalBlurPass;
		PostEffect BloomVerticalBlurPass;
		PostEffect BloomFinalPass;
		PostEffect FinalPass;

		ScreenQuad Quad;
		ParticlesRenderer ParticlesRender;

		uint32 PolygonsRendered = 0;
		uint32 OpaqueObjectsRendered = 0;
		uint32 TransparentObjectsRendered = 0;
	public:
		enum class Stage
		{
			Opaque,
			Sky,
			Transparent
		};

		enum class PostEffectResolution
		{
			Full,
			Half,
			Quad
		};
	public:
		iVector2 ContextSize;
		bool EditMode = false;

		float Exposure = 1.3f;
		float Gamma = 1.5f;

		bool BloomEnable = true;
		float BloomTreshold = 0.8f;
		float BloomIntensity = 0.5f;
		float BloomRadius = 1.0f;
		int BloomIterations = 2;
		PostEffectResolution BloomResolution = PostEffectResolution::Quad;
	private:
		void CalculateLights(const Vector3& Position, int32(&Lights)[4]);

		void RenderBloom();
		void RenderIcons();
	public:
		Renderer();
		
		void SetViewport(const iVector2& Origin, const iVector2& Size);
		void SetMainCamera(const Camera& InCamera);
		void SetSky(Skybox* InSky);
		void SetScene(Scene* InScn);

		uint32 GetPolygonsRendered() const;
		uint32 GetOpaqueObjectsRendered() const;
		uint32 GetTransparentObjectsRendered() const;

		virtual void SetRenderList(std::vector<SmartPointer<GameObject>>* List);
		virtual void SetLightsList(std::vector<Light*>* List);
		virtual void CompileLists();
		virtual void SortLists();

		virtual void RenderOpaqueStage();
		virtual void RenderSkyStage();
		virtual void RenderTransparentStage();
		virtual void Render(Stage RenderStage);
		virtual void Render();

		Texture* GetFramebufferTexture() const;

		~Renderer();
	};

}


