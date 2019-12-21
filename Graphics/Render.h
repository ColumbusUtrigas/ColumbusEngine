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

#include <Graphics/Postprocess/AutoExposure.h>
#include <Graphics/Postprocess/Bloom.h>
#include <Graphics/Postprocess/Vignette.h>

#include <Editor/Grid.h>
#include <Editor/Gizmo.h>

#include <array>

namespace Columbus
{

	enum class RenderPass
	{
		Opaque = 0,
		Sky,
		Transparent,
		Postprocess
	};

	enum class TonemappingType
	{
		Simple = 0,
		Filmic,
		ACES,
		RomBinDaHouse,
		Uncharted
	};

	enum class AntialiasingType
	{
		No = 0,
		FXAA,
		MSAA_2X,
		MSAA_4X,
		MSAA_8X,
		MSAA_16X,
		MSAA_32X
	};

	class Renderer
	{
	protected:
		struct OpaqueRenderData
		{
			Mesh* Object;
			Material* Mat;
			uint32 Index; // Index of GameObject in array
			//int32 Lights[4] = { -1, -1, -1, -1 };

			OpaqueRenderData(Mesh* InObject, uint32 InIndex, Material* InMat) :
				Object(InObject),
				Index(InIndex),
				Mat(InMat) {}
		};

		struct TransparentRenderData
		{
			Mesh* MeshObject;
			ParticleEmitterCPU* Particles;
			Material* Mat;
			uint32 Index; // Index of GameObject in array
			//int32 Lights[4] = { -1, -1, -1, -1 };

			TransparentRenderData(Mesh* InMesh, Material* Mat, uint32 InIndex) :
				MeshObject(InMesh),
				Particles(nullptr),
				Mat(Mat),
				Index(InIndex) {}

			TransparentRenderData(ParticleEmitterCPU* CPU, Material* Mat, uint32 InIndex) :
				MeshObject(nullptr),
				Particles(CPU),
				Mat(Mat),
				Index(InIndex) {}
		};
	protected:
		std::vector<SmartPointer<GameObject>>* RenderList;
		std::vector<Light*>* LightsList;
		std::vector<std::pair<uint32, Light*>> LightsPairs;

		std::vector<OpaqueRenderData> OpaqueObjects;
		std::vector<OpaqueRenderData> ShadowsObjects;
		std::vector<TransparentRenderData> TransparentObjects;
		
		iVector2 ViewportOrigin;
		iVector2 ViewportSize;

		Camera MainCamera;
		Skybox* Sky = nullptr;
		Scene* Scn = nullptr;

		PostEffect Base, BaseMSAA;
		std::array<PostEffect, 2> Post;
		PostEffect Final;

		ScreenQuad Quad;
		ParticlesRenderer ParticlesRender;

		Grid _Grid;
		Gizmo _Gizmo;

		uint32 PolygonsRendered = 0;
		uint32 OpaqueObjectsRendered = 0;
		uint32 TransparentObjectsRendered = 0;

		float DeltaTime = 0.0f;
	public:
		bool EnableMousePicking = false;
		Vector2 MousePickingPosition;
		GameObject* PickedObject = nullptr;

		iVector2 ContextSize;
		bool DrawIcons = false;
		bool DrawGrid = false;
		bool DrawGizmo = false;

		float Gamma = 2.2f;
		float Exposure = 1.0f;
		TonemappingType Tonemapping = TonemappingType::Simple;

		AntialiasingType Antialiasing = AntialiasingType::No;

		PostprocessAutoExposure AutoExposure{Quad};
		PostprocessBloom Bloom{Quad};
		PostprocessVignette Vignette{Quad};
	private:
		void CalculateLights(const Vector3& Position, int32(&Lights)[4]);

		void RenderIcons();
	public:
		Renderer();
		
		void SetViewport(const iVector2& Origin, const iVector2& Size);
		void SetMainCamera(const Camera& InCamera);
		Camera GetMainCamera() const { return MainCamera; }
		void SetSky(Skybox* InSky);
		void SetScene(Scene* InScn);
		void SetDeltaTime(float Delta);

		uint32 GetPolygonsRendered() const;
		uint32 GetOpaqueObjectsRendered() const;
		uint32 GetTransparentObjectsRendered() const;

		void SetRenderList(std::vector<SmartPointer<GameObject>>* List);
		void SetLightsList(std::vector<Light*>* List);
		void CompileLists();
		void SortLists();

		void RenderShadows(const iVector2& ShadowMapSize);
		void RenderOpaque();
		void RenderSky();
		void RenderTransparent();
		void RenderPostprocess();
		void Render(RenderPass Pass);
		void Render();

		Texture* GetFramebufferTexture() const;

		~Renderer();
	};

}


