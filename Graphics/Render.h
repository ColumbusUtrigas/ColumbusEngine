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

	enum class BloomResolutionType
	{
		Quad = 0,
		Half,
		Full
	};

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

		PostEffect Base, BaseMSAA;
		PostEffect Post1, Post2;
		PostEffect Final;

		ScreenQuad Quad;
		ParticlesRenderer ParticlesRender;

		uint32 PolygonsRendered = 0;
		uint32 OpaqueObjectsRendered = 0;
		uint32 TransparentObjectsRendered = 0;
	public:
		iVector2 ContextSize;
		bool EditMode = false;

		float Gamma = 2.2f;
		float Exposure = 1.0f;
		TonemappingType Tonemapping = TonemappingType::Simple;

		AntialiasingType Antialiasing = AntialiasingType::No;

		bool BloomEnable = true;
		float BloomTreshold = 1.0f;
		float BloomIntensity = 0.5f;
		float BloomRadius = 1.0f;
		int BloomIterations = 2;
		BloomResolutionType BloomResolution = BloomResolutionType::Quad;

		bool VignetteEnable = false;
		Vector3 VignetteColor;
		Vector2 VignetteCenter = Vector2(0.5f);
		float VignetteIntensity = 1.0f;
		float VignetteSmoothness = 0.2f;
		float VignetteRadius = 0.6f;
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

		void SetRenderList(std::vector<SmartPointer<GameObject>>* List);
		void SetLightsList(std::vector<Light*>* List);
		void CompileLists();
		void SortLists();

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


