#pragma once

#include <Graphics/DebugRender.h>
#include <Graphics/Mesh.h>
#include <Graphics/Camera.h>
#include <Graphics/Skybox.h>
#include <Graphics/Light.h>
#include <Graphics/Particles/ParticleEmitterCPU.h>
#include <Graphics/Billboard.h>
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

	struct RenderEntity
	{
		Material* Mat;
		Transform* Tran;
		Mesh* MeshObj;
		ParticleEmitterCPU* ParticlesCPU;
		Billboard* Bill;

		RenderEntity(Material* Mat, Transform& Tran, Mesh* Mesh, ParticleEmitterCPU* ParticlesCPU, Billboard* Bill) :
			Mat(Mat), Tran(&Tran), MeshObj(Mesh), ParticlesCPU(ParticlesCPU), Bill(Bill) {}
	};

	class Renderer
	{
	protected:
		std::vector<SmartPointer<GameObject>>* RenderList;
		std::vector<Light> LightsList;

		std::vector<RenderEntity> OpaqueEntities;
		std::vector<RenderEntity> TransparentEntities;
		std::vector<RenderEntity> ShadowsObjects;
		
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

		Editor::Grid _Grid;
		Editor::Gizmo _Gizmo;

		uint32 PolygonsRendered = 0;
		uint32 OpaqueObjectsRendered = 0;
		uint32 TransparentObjectsRendered = 0;

		float DeltaTime = 0.0f;
		bool IsEditor = false;
	public:
		iVector2 ContextSize;
		bool DrawIcons = false;
		bool DrawGrid = false;
		bool DrawGizmo = false;

		float Gamma = 2.2f;
		float Exposure = 1.0f;
		float Saturation = 1.0f;
		float Hue = 0.0f;
		float Temperature = 66.0f;
		Vector3 Lift;
		Vector3 Gain;
		Vector3 Offset;
		TonemappingType Tonemapping = TonemappingType::Simple;

		AntialiasingType Antialiasing = AntialiasingType::No;

		PostprocessAutoExposure AutoExposure{Quad};
		PostprocessBloom Bloom{Quad};
		PostprocessVignette Vignette{Quad};
	private:
		void RenderIcons();
	public:
		Renderer();
		
		void SetIsEditor(bool b) { IsEditor = b; }
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
		void CompileLists();
		void SortLists();

		void RenderShadows(const iVector2& ShadowMapSize);
		void RenderOpaque();
		void RenderSky();
		void RenderTransparent();
		void RenderDebug();
		void RenderFlares();
		void RenderPostprocess();
		void Render(RenderPass Pass);
		void Render();

		Texture* GetFramebufferTexture() const;

		~Renderer();
	};

}


