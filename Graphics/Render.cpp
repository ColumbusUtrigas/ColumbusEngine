#include <Graphics/Render.h>
#include <Graphics/Device.h>
#include <Graphics/RenderState.h>
#include <Scene/ComponentMeshRenderer.h>
#include <Scene/ComponentParticleSystem.h>
#include <Scene/ComponentBillboard.h>
#include <Scene/ComponentLight.h>
#include <Scene/Component.h>
#include <Math/Frustum.h>
#include <GL/glew.h>
#include <algorithm>
#include <tuple>
#include <cstddef>

#define STB_RECT_PACK_IMPLEMENTATION
#include <stb_rect_pack.h>

#include <Profiling/Profiling.h>

#include <Graphics/OpenGL/ShaderOpenGL.h>
#include <Graphics/OpenGL/FramebufferOpenGL.h>

#include <Graphics/Postprocess/GaussBlur.h>

namespace Columbus
{
	Texture* Blob;
	ShaderProgram* Prefilter;

	RenderState State;

	struct
	{
		struct
		{
			Vector3 color; float range;
			Vector3 pos; float innerCutoff;
			Vector3 dir; float outerCutoff;
			int type;
			int hasShadow;
			float pad[2];

			Matrix lightView;
			Vector4 shadowRect;
		} lights[128];

		int count;
	} lightingUboData;

	Renderer::Renderer()
	{
		Blob = gDevice->CreateTexture();
		Blob->Load("Data/Textures/blob.png");

		Prefilter = gDevice->GetDefaultShaders()->GaussBlur.get();

		BaseMSAA.ColorTexturesEnablement[0] = true;
		BaseMSAA.ColorTexturesEnablement[1] = true;
		BaseMSAA.ColorTexturesFormats[0] = TextureFormat::R11G11B10F; // Color texture
		BaseMSAA.ColorTexturesFormats[1] = TextureFormat::R11G11B10F; // Normal texture
		BaseMSAA.ColorTexturesMipmaps[0] = false;
		BaseMSAA.ColorTexturesMipmaps[1] = false;
		BaseMSAA.DepthTextureEnablement = true;

		Base.ColorTexturesEnablement[0] = true;
		Base.ColorTexturesEnablement[1] = true;
		Base.ColorTexturesFormats[0] = TextureFormat::R11G11B10F; // Color texture
		Base.ColorTexturesFormats[1] = TextureFormat::R11G11B10F; // Normal texture
		Base.ColorTexturesMipmaps[0] = false;
		Base.ColorTexturesMipmaps[1] = false;
		Base.DepthTextureEnablement = true;

		for (auto& post : Post)
		{
			post.ColorTexturesFormats[0] = TextureFormat::R11G11B10F;
			post.ColorTexturesFormats[1] = TextureFormat::R11G11B10F;
		}

		Final.ColorTexturesEnablement[0] = true;
	}

	void Renderer::SetViewport(const iVector2& Origin, const iVector2& Size)
	{
		ViewportOrigin = Origin;
		ViewportSize = Size;
	}

	void Renderer::SetMainCamera(const Camera& InCamera)
	{
		MainCamera = InCamera;
		State.SetMainCamera(MainCamera);
	}

	void Renderer::SetSky(Skybox* InSky)
	{
		Sky = InSky;
	}

	void Renderer::SetScene(Scene* InScn)
	{
		Scn = InScn;
	}

	void Renderer::SetDeltaTime(float Delta)
	{
		DeltaTime = Delta;
	}

	uint32 Renderer::GetPolygonsRendered() const
	{
		return PolygonsRendered;
	}

	uint32 Renderer::GetOpaqueObjectsRendered() const
	{
		return OpaqueObjectsRendered;
	}

	uint32 Renderer::GetTransparentObjectsRendered() const
	{
		return TransparentObjectsRendered;
	}

	void Renderer::SetRenderList(std::vector<SmartPointer<GameObject>>* List)
	{
		RenderList = List;
	}

	void Renderer::CompileLists()
	{
		PROFILE_CPU(ProfileModule::Culling);

		OpaqueEntities.clear();
		TransparentEntities.clear();
		ShadowsObjects.clear();
		LightsList.clear();

		Frustum ViewFrustum(MainCamera.GetViewProjection());

		if (RenderList != nullptr)
		{
			uint32 Counter = 0;
			for (auto& Object : *RenderList)
			{
				if (Object->Enable)
				{
					auto MeshRenderer = (ComponentMeshRenderer*)Object->GetComponent(Component::Type::MeshRenderer);
					auto ParticleSystem = (ComponentParticleSystem*)Object->GetComponent(Component::Type::ParticleSystem);
					auto Bill = Object->GetComponent<ComponentBillboard>();
					auto Light = Object->GetComponent<ComponentLight>();

					if (Light != nullptr)
					{
						LightsList.push_back(Light->GetLight());
					}

					if (MeshRenderer != nullptr)
					{
						auto Mesh = MeshRenderer->GetMesh();

						if (Mesh != nullptr)
						{
							if (Object->materials.empty()) continue;

							for (int i = 0; i < Mesh->SubMeshes.size(); i++)
							{
								auto& mesh = Mesh->SubMeshes[i];
								if (Object->materials.size() > i/* && Object->materials[i] != nullptr*/)
								{
									auto mat = Object->materials[i];
									static Material* defaultMat = new Material();
									defaultMat->SetShader(gDevice->GetDefaultShaders()->Error.get());
									defaultMat->Name = "Error";
									if (mat == nullptr)
										mat = defaultMat;

									if (ViewFrustum.Check(mesh->GetBoundingBox() * Object->transform.GetMatrix()))
									{
										if (mat->Transparent)
											TransparentEntities.emplace_back(mat, Object->transform, mesh, nullptr, nullptr);
										else
											OpaqueEntities.emplace_back(mat, Object->transform, mesh, nullptr, nullptr);
									}

									if (!mat->Transparent)
										ShadowsObjects.emplace_back(mat, Object->transform, mesh, nullptr, nullptr);
								}
							}
						}
					}

					if (ParticleSystem != nullptr)
					{
						TransparentEntities.emplace_back(Object->materials[0], Object->transform, nullptr, &ParticleSystem->Emitter, nullptr);
					}

					if (Bill != nullptr)
					{
						auto mat = Object->materials.empty() ? nullptr : Object->materials[0];
						TransparentEntities.emplace_back(mat, Object->transform, nullptr, nullptr, &Bill->GetBillboard());
					}
				}

				Counter++;
			}
		}
	}

	void Renderer::SortLists()
	{
		auto opaque_sort = [&](const RenderEntity& A, const RenderEntity& B)->bool {
			return MainCamera.Pos.LengthSquare(A.Tran->Position) < MainCamera.Pos.LengthSquare(B.Tran->Position);
		};

		auto transparent_sort = [&](const RenderEntity& A, const RenderEntity& B)->bool {
			return MainCamera.Pos.LengthSquare(A.Tran->Position) > MainCamera.Pos.LengthSquare(B.Tran->Position);
		};

		std::sort(OpaqueEntities.begin(), OpaqueEntities.end(), opaque_sort);
		std::sort(OpaqueEntities.begin(), OpaqueEntities.end(), transparent_sort);
	}

	void Renderer::RenderShadows(const iVector2& ShadowMapSize)
	{
		if (ShadowsObjects.empty()) return;

		stbrp_context context;
		int num_rects = 0;
		stbrp_rect rects[128];

		constexpr int num_nodes = 128;
		stbrp_node nodes[num_nodes];

		for (int i = 0; i < lightingUboData.count; i++)
		{
			if (lightingUboData.lights[i].hasShadow)
			{
				rects[num_rects].id = i;
				rects[num_rects].w = 1024;
				rects[num_rects].h = 1024;
				num_rects++;
			}
		}

		float fw = 1.0f / ShadowMapSize.X;
		float fh = 1.0f / ShadowMapSize.Y;

		stbrp_init_target(&context, ShadowMapSize.X, ShadowMapSize.Y, nodes, num_nodes);
		stbrp_pack_rects(&context, rects, num_rects);

		gDevice->BeginMarker("Shadows Render");
		for (int i = 0; i < num_rects; i++)
		{
			int id = rects[i].id;

			float fov = Math::Degrees(lightingUboData.lights[id].outerCutoff * 2);

			glViewport(rects[i].x, rects[i].y, rects[i].w, rects[i].h);
			Camera lightCam;
			lightCam.Pos = lightingUboData.lights[id].pos;
			lightCam.SetTarget(lightingUboData.lights[id].dir + lightingUboData.lights[id].pos);
			lightCam.Perspective(fov, 1, 0.1f, 1000);
			lightCam.Update();

			lightingUboData.lights[id].lightView = lightCam.GetViewProjection();
			lightingUboData.lights[id].shadowRect = Vector4(rects[i].x * fw, rects[i].y * fh, rects[i].w * fw, rects[i].h * fh);

			State.SetMainCamera(lightCam);

			for (auto& Object : ShadowsObjects)
			{
				Material& Mat = *Object.Mat;
				ShaderProgram* CurrentShader = Mat.GetShader();

				if (CurrentShader != nullptr)
				{
					gDevice->SetShader(Mat.GetShader());
					gDevice->OMSetDepthStencilState(Mat.DSS.get(), 0);
					gDevice->OMSetBlendState(Mat.BS.get(), nullptr, 0xFFFFFFFF);

					State.SetCulling(Material::Cull::No);
					//State.SetDepthTesting(Mat.DepthTesting);
					//State.SetDepthWriting(Mat.DepthWriting);
					State.SetMaterial(Mat, Object.Tran->GetMatrix(), Sky, true);

					Object.MeshObj->Bind();

					PolygonsRendered += Object.MeshObj->Render();
					OpaqueObjectsRendered++;
				}
			}
		}
		gDevice->EndMarker();
	}

	void Renderer::RenderOpaque()
	{
		PROFILE_GPU(ProfileModuleGPU::OpaqueStage);
		gDevice->BeginMarker("Opaque Render");

		for (auto& Ent : OpaqueEntities)
		{
			if (Ent.Mat == nullptr) continue;
			Material& Mat = *Ent.Mat;
			ShaderProgram* CurrentShader = Mat.GetShader();

			if (CurrentShader == nullptr)
				CurrentShader = gDevice->GetDefaultShaders()->Error.get();

			if (CurrentShader != nullptr)
			{
				gDevice->OMSetDepthStencilState(Mat.DSS.get(), 0);
				gDevice->OMSetBlendState(Mat.BS.get(), nullptr, 0xFFFFFFFF);
				gDevice->RSSetState(Mat.RS.get());
				gDevice->SetShader(Mat.GetShader());

				State.SetMaterial(Mat, Ent.Tran->GetMatrix(), Sky);
				Ent.MeshObj->Bind();

				PolygonsRendered += Ent.MeshObj->Render();
				OpaqueObjectsRendered++;
			}
		}
		gDevice->EndMarker();
	}

	void Renderer::RenderSky()
	{
		if (Sky != nullptr)
		{
			PROFILE_GPU(ProfileModuleGPU::SkyStage);
			gDevice->BeginMarker("Sky Render");
			BlendStateDesc BSD;
			DepthStencilStateDesc DSSD;
			RasterizerStateDesc RSD;

			BSD.RenderTarget[0].BlendEnable = false;

			DSSD.DepthEnable = true;
			DSSD.DepthWriteMask = true;
			DSSD.DepthFunc = ComparisonFunc::LEqual;

			RSD.Cull = CullMode::Back;
			RSD.FrontCounterClockwise = true;
			RSD.Fill = FillMode::Solid;

			static BlendState* BS;
			static DepthStencilState* DSS;
			static RasterizerState* RS;

			static bool bsr = gDevice->CreateBlendState(BSD, &BS);
			static bool dssr = gDevice->CreateDepthStencilState(DSSD, &DSS);
			static bool rsr = gDevice->CreateRasterizerState(RSD, &RS);

			gDevice->OMSetBlendState(BS, nullptr, 0xFFFFFFFF);
			gDevice->OMSetDepthStencilState(DSS, 0);
			gDevice->RSSetState(RS);

			Sky->Render();
			gDevice->EndMarker();
		}
	}

	std::string billboard =
		R"(
#shader vertex
#attribute float3 aPos 0
#uniform float4x4 uViewProjection
#uniform float4x4 uModel
#uniform float4x4 uRot

out float3 varPos;

void main(void)
{
	varPos = float3(float4(aPos, 1.0) * uModel);
	SV_Position = uViewProjection * float4(varPos, 1);
}

#shader pixel
in float3 varPos;

void main(void)
{
	RT0 = float4(1,1,1,1);
}
)";

	void RenderBillboard(Camera Cam, const Billboard& Bill, const Transform& Tran)
	{
		static ShaderProgram* prog;
		static Mesh* plane;
		static bool first = true;
		if (first)
		{
			prog = gDevice->CreateShaderProgram();
			prog->LoadFromMemory(billboard.c_str());
			prog->Compile();

			plane = gDevice->CreateMesh();
			plane->Load("Data/Meshes/Plane.obj");
			first = false;
		}

		Matrix model;
		Matrix rot;

		switch (Bill.Rotate)
		{
		//case Billboard::LocalX: rot.LookAt(Tran.Position, { 0,Cam.Pos.Y,Cam.Pos.Z }, { 1,0,0 }); break;
		case Billboard::LocalY: rot.LookAt(Tran.Position, { Cam.Pos.X,0,Cam.Pos.Z }, { 0,1,0 }); break;
		//case Billboard::LocalZ: rot.LookAt(Tran.Position, { Cam.Pos.X,0,Cam.Pos.Z }, { 0,1,0 }); break;
		}

		model.Scale(Tran.Scale);
		model = rot * model;
		model = Tran.Rotation.ToMatrix() * model;
		model.Translate(Tran.Position);

		gDevice->SetShader(prog);
		static_cast<ShaderProgramOpenGL*>(prog)->SetUniform("uViewProjection", false, Cam.GetViewProjection());
		static_cast<ShaderProgramOpenGL*>(prog)->SetUniform("uModel", false, model);
		static_cast<ShaderProgramOpenGL*>(prog)->SetUniform("uRot", false, rot);

		plane->SubMeshes[0]->Bind();
		plane->SubMeshes[0]->Render();
	}

	void Renderer::RenderTransparent()
	{
		PROFILE_GPU(ProfileModuleGPU::TransparentStage);

		if (TransparentEntities.size() != 0)
		{
			gDevice->BeginMarker("Transparent Render");
			BlendStateDesc BSD;
			BlendState* BS;
			BSD.RenderTarget[0].BlendEnable = true;
			BSD.RenderTarget[0].SrcBlend = Blend::SrcAlpha;
			BSD.RenderTarget[0].DestBlend = Blend::InvSrcAlpha;
			BSD.RenderTarget[0].SrcBlendAlpha = Blend::SrcAlpha;
			BSD.RenderTarget[0].DestBlendAlpha = Blend::InvSrcAlpha;

			gDevice->CreateBlendState(BSD, &BS);

			for (auto& Object : TransparentEntities)
			{
				auto Mat = Object.Mat;
				ShaderProgramOpenGL* CurrentShader = static_cast<ShaderProgramOpenGL*>(Mat != nullptr ? Mat->GetShader() : nullptr);
				Mesh* CurrentMesh = Object.MeshObj;

				if (CurrentMesh != nullptr)
				{
					if (Mat == nullptr) continue;

					if (CurrentShader == nullptr)
						CurrentShader = static_cast<ShaderProgramOpenGL*>(gDevice->GetDefaultShaders()->Error.get());

					if (CurrentShader != nullptr)
					{
						gDevice->SetShader(CurrentShader);
						gDevice->OMSetDepthStencilState(Mat->DSS.get(), 0);
						gDevice->OMSetBlendState(BS, nullptr, 0xFFFFFFFF);
						gDevice->RSSetState(Mat->RS.get());
						State.SetMaterial(*Mat, Object.Tran->GetMatrix(), Sky);
						CurrentMesh->Bind();

						int32 Transparent = CurrentShader->GetFastUniform("Transparent");

						if (Mat->Culling == Material::Cull::No)
						{
							State.SetDepthWriting(true);
							CurrentShader->SetUniform(Transparent, 0);
							State.SetCulling(Material::Cull::No);
							CurrentMesh->Render();

							State.SetDepthWriting(false);
							CurrentShader->SetUniform(Transparent, 1);
							State.SetCulling(Material::Cull::Front);
							CurrentMesh->Render();
							State.SetCulling(Material::Cull::Back);
							PolygonsRendered += CurrentMesh->Render();
						}
						else
						{
							State.SetCulling(Mat->Culling);

							State.SetDepthWriting(true);
							CurrentShader->SetUniform(Transparent, 0);
							CurrentMesh->Render();

							State.SetDepthWriting(false);
							CurrentShader->SetUniform(Transparent, 1);
							PolygonsRendered += CurrentMesh->Render();
						}

						TransparentObjectsRendered++;
					}
				}

				if (Object.ParticlesCPU != nullptr)
				{
					if (Mat == nullptr) continue;

					gDevice->SetShader(CurrentShader);
					ParticlesRender.SetDepthBuffer(Base.DepthTexture);
					ParticlesRender.Render(*Object.ParticlesCPU, MainCamera, *Mat);
				}

				if (Object.Bill != nullptr)
				{
					RenderBillboard(MainCamera, *Object.Bill, *Object.Tran);
				}
			}
			gDevice->EndMarker();
		}
	}

	void Renderer::RenderDebug()
	{
		auto EditorToolsShader = static_cast<ShaderProgramOpenGL*>(gDevice->GetDefaultShaders()->EditorTools.get());

		if (!Graphics::gDebugRender.objects.empty())
		{
			gDevice->BeginMarker("Debug Draw");

			EditorToolsShader->Bind();
			EditorToolsShader->SetUniform("ViewProjection", false, MainCamera.GetViewProjection());
			EditorToolsShader->SetUniform("CameraPos", MainCamera.Pos);
			EditorToolsShader->SetUniform("UseDistanceFade", 0);

			for (auto& obj : Graphics::gDebugRender.objects)
			{
				gDevice->RSSetState(obj.rs);

				EditorToolsShader->SetUniform("Color", obj.color);
				EditorToolsShader->SetUniform("Model", false, obj.transform);
				obj.mesh->SubMeshes[0]->Bind();
				obj.mesh->SubMeshes[0]->Render();
			}

			EditorToolsShader->Unbind();
			gDevice->EndMarker();
		}
	}

	void Renderer::RenderFlares()
	{

	}

	void Renderer::RenderPostprocess()
	{

	}

	void Renderer::Render(RenderPass Pass)
	{
		if (RenderList != nullptr)
		{
			switch (Pass)
			{
			case RenderPass::Opaque:      RenderOpaque();      break;
			case RenderPass::Sky:         RenderSky();         break;
			case RenderPass::Transparent: RenderTransparent(); break;
			case RenderPass::Postprocess: RenderPostprocess(); break;
			}
		}
	}

	void Renderer::RenderIcons()
	{
		auto Icon = gDevice->GetDefaultShaders()->Icon.get();

		static int IconTextureID = ((ShaderProgramOpenGL*)(Icon))->GetFastUniform("Texture");
		static int IconPosID = ((ShaderProgramOpenGL*)(Icon))->GetFastUniform("Pos");
		static int IconSizeID = ((ShaderProgramOpenGL*)(Icon))->GetFastUniform("Size");

		auto DrawIcon = [&](Vector4 Coords)
		{
			float InvDistance = 1.0f / Coords.XYZ().Length(MainCamera.Pos) * 0.5f;
			InvDistance = Math::Clamp(InvDistance, 0.05f, 0.5f);
			Coords = MainCamera.GetViewProjection() * Coords;

			if (Coords.W > 0.0f)
			{
				Coords /= Coords.W;

				float Aspect = (float)ContextSize.X / (float)ContextSize.Y;
				Vector2 Size = InvDistance / Vector2(Aspect, 1);

				((ShaderProgramOpenGL*)(Icon))->SetUniform(IconPosID, Coords.XY());
				((ShaderProgramOpenGL*)(Icon))->SetUniform(IconSizeID, Size);
				Quad.Render();
			}
		};

		((ShaderProgramOpenGL*)(Icon))->Bind();

		static BlendStateDesc BSD;
		static BlendState* BS;
		BSD.RenderTarget[0].BlendEnable = true;
		BSD.RenderTarget[0].SrcBlend = Blend::SrcAlpha;
		BSD.RenderTarget[0].DestBlend = Blend::InvSrcAlpha;
		BSD.RenderTarget[0].SrcBlendAlpha = Blend::SrcAlpha;
		BSD.RenderTarget[0].DestBlendAlpha = Blend::InvSrcAlpha;

		static bool bsr = gDevice->CreateBlendState(BSD, &BS);
		gDevice->OMSetBlendState(BS, nullptr, 0xFFFFFFFF);

		((ShaderProgramOpenGL*)(Icon))->SetUniform(IconTextureID, (TextureOpenGL*)gDevice->GetDefaultTextures()->IconSun.get(), 0);
		for (const auto& Elem : LightsList)
		{
			if (Elem.Type == Light::Directional)
				DrawIcon(Vector4(Elem.Pos, 1));
		}

		((ShaderProgramOpenGL*)(Icon))->SetUniform(IconTextureID, (TextureOpenGL*)gDevice->GetDefaultTextures()->IconLamp.get(), 0);
		for (const auto& Elem : LightsList)
		{
			if (Elem.Type == Light::Point)
				DrawIcon(Vector4(Elem.Pos, 1));
		}

		((ShaderProgramOpenGL*)(Icon))->SetUniform(IconTextureID, (TextureOpenGL*)gDevice->GetDefaultTextures()->IconFlashlight.get(), 0);
		for (const auto& Elem : LightsList)
		{
			if (Elem.Type == Light::Spot)
				DrawIcon(Vector4(Elem.Pos, 1));
		}

		((ShaderProgramOpenGL*)(Icon))->SetUniform(IconTextureID, (TextureOpenGL*)gDevice->GetDefaultTextures()->IconAudio.get(), 0);
		for (auto Elem : Scn->Audio.Mixer.Sources)
		{
			if ((bool)Elem)
				DrawIcon(Vector4(Elem->Position, 1));
		}

		((ShaderProgramOpenGL*)(Icon))->SetUniform(IconTextureID, (TextureOpenGL*)gDevice->GetDefaultTextures()->IconParticles.get(), 0);
		for (const auto& Elem : TransparentEntities)
		{
			if (Elem.ParticlesCPU != nullptr)
				DrawIcon(Vector4(Elem.Tran->Position, 1));
		}

		((ShaderProgramOpenGL*)(Icon))->Unbind();
	}

	void Renderer::Render()
	{
		struct _UBO_Data
		{
			struct
			{
				float min, max;
				float speedUp, speedDown;
				float deltaTime;
				float pad[59]; // because nvidia requires 256-byte alignment
			} autoExposure;

			struct
			{
				float exposure, gamma;
				int type;
				int ae_enable;
				float pad[60];
			} tonemap;

			struct
			{
				Vector3 color;
				float intensity;
				Vector2 center;
				float smoothness;
				float radius;
				float pad[56];
			} vignette;
		} uboData;

		uboData.autoExposure.min = AutoExposure.Min;
		uboData.autoExposure.max = AutoExposure.Max;
		uboData.autoExposure.speedUp = AutoExposure.SpeedUp;
		uboData.autoExposure.speedDown = AutoExposure.SpeedDown;
		uboData.autoExposure.deltaTime = DeltaTime;

		uboData.tonemap.exposure = Exposure;
		uboData.tonemap.gamma = Gamma;
		uboData.tonemap.type = static_cast<int>(Tonemapping);
		uboData.tonemap.ae_enable = AutoExposure.Enabled;

		uboData.vignette.color = Vignette.Color;
		uboData.vignette.intensity = Vignette.Intensity;
		uboData.vignette.center = Vignette.Center;
		uboData.vignette.smoothness = Vignette.Smoothness;
		uboData.vignette.radius = Vignette.Radius;

		static Buffer* UBO;
		static bool uboresult = gDevice->CreateBuffer(BufferDesc(
			sizeof(uboData),
			BufferType::Uniform,
			BufferUsage::Dynamic,
			BufferCpuAccess::Write
		), nullptr, &UBO);

		void* ubomap;
		gDevice->MapBuffer(UBO, BufferMapAccess::Write, ubomap);
		memcpy(ubomap, &uboData, sizeof(uboData));
		gDevice->UnmapBuffer(UBO);

		auto defaultShaders = gDevice->GetDefaultShaders();

		auto ScreenSpaceShader = static_cast<ShaderProgramOpenGL*>(defaultShaders->ScreenSpace.get());
		auto TonemapShader     = static_cast<ShaderProgramOpenGL*>(defaultShaders->Tonemap.get());
		auto MSAAShader        = static_cast<ShaderProgramOpenGL*>(defaultShaders->ResolveMSAA.get());
		auto FXAAShader        = static_cast<ShaderProgramOpenGL*>(defaultShaders->FXAA.get());
		auto EditorToolsShader = static_cast<ShaderProgramOpenGL*>(defaultShaders->EditorTools.get());

		if (ContextSize.X == 0) ContextSize.X = 1;
		if (ContextSize.Y == 0) ContextSize.Y = 1;

		PolygonsRendered = 0;
		OpaqueObjectsRendered = 0;
		TransparentObjectsRendered = 0;

		CompileLists();
		SortLists();

		PROFILE_GPU(ProfileModuleGPU::GPU);

		bool IsFXAA = false;
		bool IsMSAA = false;

		switch (Antialiasing)
		{
		case AntialiasingType::No: break;
		case AntialiasingType::FXAA: IsFXAA = true; break;
		case AntialiasingType::MSAA_2X:  BaseMSAA.Multisampling = 2;  IsMSAA = true; break;
		case AntialiasingType::MSAA_4X:  BaseMSAA.Multisampling = 4;  IsMSAA = true; break;
		case AntialiasingType::MSAA_8X:  BaseMSAA.Multisampling = 8;  IsMSAA = true; break;
		case AntialiasingType::MSAA_16X: BaseMSAA.Multisampling = 16; IsMSAA = true; break;
		case AntialiasingType::MSAA_32X: BaseMSAA.Multisampling = 32; IsMSAA = true; break;
		}

		lightingUboData.count = 0;
		for (const auto& Light : LightsList)
		{
			auto id = lightingUboData.count;

			lightingUboData.lights[id].color = Light.Color * Light.Energy;
			lightingUboData.lights[id].pos = Light.Pos;
			lightingUboData.lights[id].dir = Light.Dir;

			lightingUboData.lights[id].range = Light.Range;
			lightingUboData.lights[id].innerCutoff = Math::Radians(Light.InnerCutoff) / 2;
			lightingUboData.lights[id].outerCutoff = Math::Radians(Light.OuterCutoff) / 2;

			lightingUboData.lights[id].type = Light.Type;
			lightingUboData.lights[id].hasShadow = (int)Light.Shadows;

			lightingUboData.count++;
		}

		static Buffer* buf;
		static bool ludbufres =
		gDevice->CreateBuffer(BufferDesc{
			sizeof(lightingUboData),
			BufferType::Uniform,
			BufferUsage::Dynamic,
			BufferCpuAccess::Write
		}, nullptr, &buf);

		GLuint BuffersAll[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		GLuint BuffersFirst[] = { GL_COLOR_ATTACHMENT0 };

		static PostEffect shadowEffect;
		shadowEffect.DepthTextureEnablement = true;
		shadowEffect.DepthTextureFlags.Filtering = Texture::Filter::Linear;

		iVector2 shadowSize = { 2048, 2048 };
		shadowEffect.Bind({0}, {0}, shadowSize);
		RenderShadows(shadowSize);

		void* data;
		gDevice->MapBuffer(buf, BufferMapAccess::Write, data);
		memcpy(data, &lightingUboData, sizeof(lightingUboData));
		gDevice->UnmapBuffer(buf);
		gDevice->BindBufferRange(buf, 0, 0, sizeof(lightingUboData));

		// RENDERING
		//
		//
		// Render to the MSAA buffer or not
		if (IsMSAA)
			BaseMSAA.Bind({0}, {0}, ContextSize);
		else
			Base.Bind({0}, {0}, ContextSize);

		State.ShadowTexture = shadowEffect.DepthTexture;
		State.SetMainCamera(MainCamera);
		ParticlesRender.UBO = buf;

		RenderOpaque();
		glDrawBuffers(1, BuffersFirst);
		RenderSky();
		glDrawBuffers(2, BuffersAll);
		//RenderTransparent();

		if (DrawGrid)
		{
			gDevice->BeginMarker("Grid Draw");
			glDrawBuffers(1, BuffersFirst);

			EditorToolsShader->Bind();
			EditorToolsShader->SetUniform("ViewProjection", false, MainCamera.GetViewProjection());
			EditorToolsShader->SetUniform("Color", Vector4(Vector3(0.5f), 1.0f));
			EditorToolsShader->SetUniform("CameraPos", MainCamera.Pos);
			EditorToolsShader->SetUniform("Model", false, Matrix(1.0f));
			EditorToolsShader->SetUniform("UseDistanceFade", 1);
			_Grid.Draw();
			EditorToolsShader->Unbind();
			gDevice->EndMarker();
		}

		// Debug draw
		glDrawBuffers(1, BuffersFirst);
		RenderDebug();

		// TODO
		gDevice->RSSetState(Graphics::gDebugRender.RS_solid);

		//
		//
		// RENDERING

		if (IsMSAA)
		{
			Base.Bind({0}, {0}, ContextSize);

			auto msaaSamples = Math::Min<int>(BaseMSAA.Multisampling, OpenGL::GetMaxColorTextureSamples());

			// Resolve RT0 (HDR Color)
			MSAAShader->Bind();
			MSAAShader->SetUniform("BaseTexture", BaseMSAA.ColorTextures[0], 0);
			MSAAShader->SetUniform("Samples", msaaSamples);
			Quad.Render();
			MSAAShader->Unbind();

			// Resolve RT1 (Normals) and Depth
			glBindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<FramebufferOpenGL*>(BaseMSAA.FB)->ID);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<FramebufferOpenGL*>(Base.FB)->ID);

			glReadBuffer(GL_COLOR_ATTACHMENT1);
			glDrawBuffer(GL_COLOR_ATTACHMENT1);
			glBlitFramebuffer(0, 0, ContextSize.X, ContextSize.Y, 0, 0, ContextSize.X, ContextSize.Y, GL_COLOR_BUFFER_BIT, GL_NEAREST);

			glReadBuffer(GL_DEPTH_ATTACHMENT);
			glDrawBuffer(GL_DEPTH_ATTACHMENT);
			glBlitFramebuffer(0, 0, ContextSize.X, ContextSize.Y, 0, 0, ContextSize.X, ContextSize.Y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

			glReadBuffer(GL_COLOR_ATTACHMENT0);
			glDrawBuffer(GL_COLOR_ATTACHMENT0);
		}

		Base.ColorTexturesMipmaps[0] = true;

		// !!!!!!!!!!!
		if (AutoExposure.Enabled)
		{
			Base.Mipmaps();
		}

		State.SetBlending(false);
		State.SetDepthWriting(false);
		State.SetCulling(Material::Cull::No);

		State.TranslucentTex = GaussBlur(Base.ColorTextures[0], ContextSize, 6);

		Base.Clear = false;
		Base.Bind({}, {}, ContextSize);
		State.ContextSize = ContextSize;
		RenderTransparent();
		Base.Clear = true;
		Base.Unbind();

		State.SetBlending(false);
		State.SetDepthWriting(false);
		State.SetCulling(Material::Cull::No);

		Texture* FinalTex = Base.ColorTextures[0];

		auto autoExposureTexture = AutoExposure.Draw(Exposure,
			Base.ColorTextures[0], ContextSize,
			UBO, offsetof(_UBO_Data, autoExposure), sizeof(uboData.autoExposure));

		Bloom.Draw(FinalTex, ContextSize, Post);

		// Final stage
		{
			PROFILE_GPU(ProfileModuleGPU::FinalStage);

			Final.Bind({}, {}, ContextSize);
			Final.ColorTextures[0]->SetMipmapLevel(0, 0);

			TonemapShader->Bind();
			TonemapShader->SetUniform("u_BaseTexture", FinalTex, 0);
			TonemapShader->SetUniform("u_AETexture", autoExposureTexture, 1);
			TonemapShader->SetUniform("u_Saturation", Saturation);
			TonemapShader->SetUniform("u_Hue", Hue);
			TonemapShader->SetUniform("u_Temperature", Temperature);
			TonemapShader->SetUniform("u_Lift", Lift);
			TonemapShader->SetUniform("u_Gain", Gain);
			TonemapShader->SetUniform("u_Offset", Offset);
			gDevice->BindBufferRange(UBO, 0, offsetof(_UBO_Data, tonemap), sizeof(uboData.tonemap));
			Quad.Render();
			TonemapShader->Unbind();

			if (IsFXAA)
			{
				Post[0].ColorTexturesEnablement[0] = true;
				Post[0].Bind({}, {}, ContextSize);

				FXAAShader->Bind();
				FXAAShader->SetUniform("BaseTexture", Final.ColorTextures[0], 0);
				FXAAShader->SetUniform("Resolution", (Vector2)ContextSize);
				Quad.Render();

				Final.FB->Bind();

				ScreenSpaceShader->Bind();
				ScreenSpaceShader->SetUniform("BaseTexture", Post[0].ColorTextures[0], 0);
				Quad.Render();
			}

			RenderFlares();

			if (DrawIcons)
			{
				RenderIcons();
			}

			if (Vignette.Enabled)
			{
				Vignette.Draw(UBO, offsetof(_UBO_Data, vignette), sizeof(uboData.vignette));
			}

			Final.Unbind();

			if (!IsEditor)
			{
				ScreenSpaceShader->Bind();
				ScreenSpaceShader->SetUniform("BaseTexture", Final.ColorTextures[0], 0);
				Quad.Render();
			}
		}


		// Lens flare rendering test, I will use it in the future

		/*static int LensFlareShaderTextureID = ((ShaderProgramOpenGL*)(LensFlareShader))->GetFastUniform("Texture");

		Vector4 Coords(Vector3(0), 1);
		Coords = MainCamera.GetViewProjection() * Coords;

		if (Coords.W != 0.0f)
			Coords /= Coords.W;

		float Aspect = (float)ContextSize.X / (float)ContextSize.Y;

		LensFlareShader->Bind();
		((ShaderProgramOpenGL*)(LensFlareShader))->SetUniform(LensFlareShaderTextureID, (TextureOpenGL*)Blob, 0);
		Quad.Render(Coords.XY(),  0.2f / Vector2(Aspect, 1));
		LensFlareShader->Unbind();*/
	}

	Texture* Renderer::GetFramebufferTexture() const
	{
		return Final.ColorTextures[0];
	}

	Renderer::~Renderer()
	{
		delete Blob;
		//delete EditorIconLamp;
	}

}


