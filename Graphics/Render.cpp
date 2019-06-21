#include <Graphics/Render.h>
#include <Graphics/Device.h>
#include <Graphics/RenderState.h>
#include <Scene/ComponentMeshRenderer.h>
#include <Scene/ComponentParticleSystem.h>
#include <Scene/Component.h>
#include <Math/Frustum.h>
#include <GL/glew.h>
#include <algorithm>

#include <Profiling/Profiling.h>

#include <Graphics/OpenGL/ShaderOpenGL.h>
#include <Graphics/OpenGL/FramebufferOpenGL.h>

namespace Columbus
{
	Texture* Blob;
	Texture* EditorIconSun;
	Texture* EditorIconLamp;
	Texture* EditorIconFlashlight;
	Texture* EditorIconAudio;
	Texture* EditorIconParticles;

	RenderState State;

	Renderer::Renderer()
	{
		Blob = gDevice->CreateTexture();
		EditorIconSun = gDevice->CreateTexture();
		EditorIconLamp = gDevice->CreateTexture();
		EditorIconAudio = gDevice->CreateTexture();
		EditorIconFlashlight = gDevice->CreateTexture();
		EditorIconParticles = gDevice->CreateTexture();

		Blob->Load("Data/Textures/blob.png");
		EditorIconSun->Load("Data/Icons/Sun.png");
		EditorIconLamp->Load("Data/Icons/Lamp.png");
		EditorIconFlashlight->Load("Data/Icons/Flashlight.png");
		EditorIconAudio->Load("Data/Icons/Audio.png");
		EditorIconParticles->Load("Data/Icons/Particles.png");

		BaseMSAA.ColorTexturesEnablement[0] = true;
		BaseMSAA.ColorTexturesEnablement[1] = true;
		BaseMSAA.ColorTexturesFormats[0] = TextureFormat::RGB16F;
		BaseMSAA.ColorTexturesMipmaps[0] = false;
		BaseMSAA.ColorTexturesMultisampling[0] = 4;
		BaseMSAA.DepthTextureEnablement = true;

		Base.ColorTexturesEnablement[0] = true;
		Base.ColorTexturesEnablement[1] = true;
		Base.ColorTexturesFormats[0] = TextureFormat::RGB16F;
		Base.ColorTexturesMipmaps[0] = false;
		Base.DepthTextureEnablement = true;

		BloomBrightPass.ColorTexturesEnablement[0] = true;
		BloomBrightPass.ColorTexturesMipmaps[0] = false;

		BloomHorizontalBlurPass.ColorTexturesEnablement[0] = true;
		BloomVerticalBlurPass.ColorTexturesEnablement[0] = true;

		BloomHorizontalBlurPass.ColorTexturesMipmaps[0] = false;
		BloomVerticalBlurPass.ColorTexturesMipmaps[0] = false;

		BloomFinalPass.ColorTexturesEnablement[0] = true;

		FinalPass.ColorTexturesEnablement[0] = true;
		//FinalPass.ColorTexturesMipmaps[0] = false;
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

	void Renderer::SetLightsList(std::vector<Light*>* List)
	{
		LightsList = List;
	}

	void Renderer::CompileLists()
	{
		PROFILE_CPU(ProfileModule::Culling);

		OpaqueObjects.clear();
		TransparentObjects.clear();

		LightsPairs.clear();

		Frustum ViewFrustum(MainCamera.GetViewProjection());

		for (uint32 i = 0; i < Scn->Lights.size(); i++)
		{
			LightsPairs.emplace_back(i, Scn->Lights[i]);
		}

		if (RenderList != nullptr)
		{
			uint32 Counter = 0;
			for (auto& Object : *RenderList)
			{
				if (Object->Enable)
				{
					ComponentMeshRenderer* MeshRenderer = (ComponentMeshRenderer*)Object->GetComponent(Component::Type::MeshRenderer);
					ComponentParticleSystem* ParticleSystem = (ComponentParticleSystem*)Object->GetComponent(Component::Type::ParticleSystem);

					if (MeshRenderer != nullptr)
					{
						auto Mesh = MeshRenderer->GetMesh();

						if (Mesh != nullptr)
						{
							if (ViewFrustum.Check(Mesh->GetBoundingBox() * Object->transform.GetMatrix()))
							{
								Object->material.ReflectionMap = Sky->GetIrradianceMap();

								if (Object->material.Transparent)
								{
									//TransparentObjects.emplace_back(Mesh, nullptr, Object->transform, Object->material);
									TransparentObjects.emplace_back(Mesh, Counter);
									CalculateLights(Object->transform.Position, TransparentObjects.back().Lights);
								}
								else
								{
									OpaqueObjects.emplace_back(Mesh, Counter);
									CalculateLights(Object->transform.Position, OpaqueObjects.back().Lights);
								}
							}
						}
					}

					if (ParticleSystem != nullptr)
					{
						auto Emitter = &ParticleSystem->Emitter;

						if (Emitter != nullptr)
						{
							//TransparentObjects.emplace_back(nullptr, Emitter, Object->transform, Object->material);
							TransparentObjects.emplace_back(Emitter, Counter);
						}
					}

					Counter++;
				}
			}
		}
	}

	void Renderer::SortLists()
	{
		auto OpaqueSorter = [&](const OpaqueRenderData& A, const OpaqueRenderData& B)->bool
		{
			return MainCamera.Pos.LengthSquare(Scn->Objects[A.Index]->transform.Position) < MainCamera.Pos.LengthSquare(Scn->Objects[B.Index]->transform.Position);
		};

		auto TransparentSorter = [&](const TransparentRenderData& A, const TransparentRenderData& B)->bool
		{
			return MainCamera.Pos.LengthSquare(Scn->Objects[A.Index]->transform.Position) > MainCamera.Pos.LengthSquare(Scn->Objects[B.Index]->transform.Position);
		};

		std::sort(OpaqueObjects.begin(), OpaqueObjects.end(), OpaqueSorter);
		std::sort(TransparentObjects.begin(), TransparentObjects.end(), TransparentSorter);
	}

	void Renderer::RenderOpaqueStage()
	{
		PROFILE_GPU(ProfileModuleGPU::OpaqueStage);

		State.Clear();

		State.SetBlending(false);

		for (auto& Object : OpaqueObjects)
		{
			SmartPointer<GameObject>& GO = Scn->Objects[Object.Index];
			Material& Mat = GO->material;
			ShaderProgram* CurrentShader = Mat.ShaderProg;
			std::vector<Light*>& Lights = Scn->Lights;
				
			if (CurrentShader != nullptr)
			{
				State.SetCulling(Mat.Culling);
				State.SetDepthTesting(Mat.DepthTesting);
				State.SetDepthWriting(Mat.DepthWriting);
				State.SetShaderProgram(Mat.GetShader());
				State.SetMaterial(Mat, GO->transform.GetMatrix(), Sky);
				State.SetLights(Lights, Object.Lights);
				State.SetMesh(Object.Object);

				PolygonsRendered += Object.Object->Render();
				OpaqueObjectsRendered++;
			}
		}
	}

	void Renderer::RenderSkyStage()
	{
		PROFILE_GPU(ProfileModuleGPU::SkyStage);

		if (Sky != nullptr)
		{
			Sky->Render();
		}
	}

	void Renderer::RenderTransparentStage()
	{
		PROFILE_GPU(ProfileModuleGPU::TransparentStage);

		if (RenderList != nullptr && TransparentObjects.size() != 0)
		{
			State.Clear();
			State.SetBlending(true);
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			for (auto& Object : TransparentObjects)
			{
				SmartPointer<GameObject>& GO = Scn->Objects[Object.Index];
				Material& Mat = GO->material;

				ShaderProgramOpenGL* CurrentShader = (ShaderProgramOpenGL*)Mat.ShaderProg;
				Mesh* CurrentMesh = Object.MeshObject;

				std::vector<Light*>& Lights = Scn->Lights;

				if (Object.MeshObject != nullptr)
				{
					if (CurrentShader != nullptr)
					{
						State.SetDepthTesting(Mat.DepthTesting);
						State.SetShaderProgram(CurrentShader);
						State.SetMaterial(Mat, GO->transform.GetMatrix(), Sky);
						State.SetLights(Lights, Object.Lights);
						CurrentMesh->Bind();

						int32 Transparent = CurrentShader->GetFastUniform("Transparent");

						if (Mat.Culling == Material::Cull::No)
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
							State.SetCulling(Mat.Culling);

							State.SetDepthWriting(true);
							CurrentShader->SetUniform(Transparent, 0);
							CurrentMesh->Render();

							State.SetDepthWriting(false);
							CurrentShader->SetUniform(Transparent, 1);
							PolygonsRendered += CurrentMesh->Render();
						}

						TransparentObjectsRendered++;

						CurrentMesh->Unbind();
					}
				}

				if (Object.Particles != nullptr)
				{
					State.SetShaderProgram(CurrentShader);
					ParticlesRender.Render(*Object.Particles, MainCamera, Mat);
				}
			}
		}

		State.SetDepthWriting(true);
	}

	void Renderer::Render(Renderer::Stage RenderStage)
	{
		if (RenderList != nullptr)
		{
			switch (RenderStage)
			{
			case Renderer::Stage::Opaque:      RenderOpaqueStage();      break;
			case Renderer::Stage::Sky:         RenderSkyStage();         break;
			case Renderer::Stage::Transparent: RenderTransparentStage(); break;
			}
		}

		State.SetCulling(Material::Cull::No);
	}

	void Renderer::CalculateLights(const Vector3& Position, int32(&Lights)[4])
	{
		static auto func = [&Position](const auto& A, const auto& B) -> bool
		{
			double ADistance = Math::Sqr(A.second->Pos.X - Position.X) + Math::Sqr(A.second->Pos.Y - Position.Y) + Math::Sqr(A.second->Pos.Z - Position.Z);
			double BDistance = Math::Sqr(B.second->Pos.X - Position.X) + Math::Sqr(B.second->Pos.Y - Position.Y) + Math::Sqr(B.second->Pos.Z - Position.Z);
			return ADistance < BDistance;
		};

		if (LightsPairs.size() >= 4)
		{
			std::sort(LightsPairs.begin(), LightsPairs.end(), func);
		}

		for (uint32 i = 0; i < 4 && i < LightsPairs.size(); i++)
		{
			Lights[i] = LightsPairs[i].first;
		}
	}

	void Renderer::RenderBloom()
	{
		PROFILE_GPU(ProfileModuleGPU::BloomStage);

		auto BloomBrightShader = gDevice->GetDefaultShaders()->BloomBright;
		auto Blur = gDevice->GetDefaultShaders()->GaussBlur;
		auto Bloom = gDevice->GetDefaultShaders()->Bloom;

		static int BrightShaderTexture = ((ShaderProgramOpenGL*)BloomBrightShader)->GetFastUniform("BaseTexture");
		static int BrightShaderTreshold = ((ShaderProgramOpenGL*)BloomBrightShader)->GetFastUniform("Treshold");

		static int BloomBlurTexture = ((ShaderProgramOpenGL*)Blur)->GetFastUniform("BaseTexture");
		static int BloomBlurHorizontal = ((ShaderProgramOpenGL*)Blur)->GetFastUniform("Horizontal");
		static int BloomBlurRadius = ((ShaderProgramOpenGL*)Blur)->GetFastUniform("Radius");

		static int BloomFinalPassBaseTexture = ((ShaderProgramOpenGL*)Bloom)->GetFastUniform("BaseTexture");
		static int BloomFinalPassVerticalBlur = ((ShaderProgramOpenGL*)Bloom)->GetFastUniform("Blur");
		static int BloomFinalPassIntensity = ((ShaderProgramOpenGL*)Bloom)->GetFastUniform("Intensity");

		BloomBrightPass.Bind({}, {0}, ContextSize);

		((ShaderProgramOpenGL*)BloomBrightShader)->Bind();
		((ShaderProgramOpenGL*)BloomBrightShader)->SetUniform(BrightShaderTexture, (TextureOpenGL*)Base.ColorTextures[0], 0);
		((ShaderProgramOpenGL*)BloomBrightShader)->SetUniform(BrightShaderTreshold, BloomTreshold);
		Quad.Render();

		BloomBrightPass.Unbind();

		((ShaderProgramOpenGL*)Blur)->Bind();
		((ShaderProgramOpenGL*)Blur)->SetUniform(BloomBlurRadius, BloomRadius);

		iVector2 Resolution;

		switch (BloomResolution)
		{
		case PostEffectResolution::Full: Resolution = ContextSize;     break;
		case PostEffectResolution::Half: Resolution = ContextSize / 2; break;
		case PostEffectResolution::Quad: Resolution = ContextSize / 4; break;
		}

		if (Resolution.X == 0) Resolution.X = 1;
		if (Resolution.Y == 0) Resolution.Y = 1;

		for (int i = 0; i < BloomIterations; i++)
		{
			auto Horiz = i == 0 ? BloomBrightPass.ColorTextures[0] : BloomVerticalBlurPass.ColorTextures[0];

			BloomHorizontalBlurPass.Bind({}, {0}, Resolution);
			((ShaderProgramOpenGL*)Blur)->SetUniform(BloomBlurTexture, (TextureOpenGL*)Horiz, 0);
			((ShaderProgramOpenGL*)Blur)->SetUniform(BloomBlurHorizontal, 1);
			Quad.Render();
			BloomHorizontalBlurPass.Unbind();

			BloomVerticalBlurPass.Bind({}, {0}, Resolution);
			((ShaderProgramOpenGL*)Blur)->SetUniform(BloomBlurTexture, (TextureOpenGL*)BloomHorizontalBlurPass.ColorTextures[0], 0);
			((ShaderProgramOpenGL*)Blur)->SetUniform(BloomBlurHorizontal, 0);
			Quad.Render();
			BloomVerticalBlurPass.Unbind();
		}

		BloomFinalPass.Bind({}, {0}, ContextSize);

		((ShaderProgramOpenGL*)Bloom)->Bind();
		((ShaderProgramOpenGL*)Bloom)->SetUniform(BloomFinalPassBaseTexture, (TextureOpenGL*)Base.ColorTextures[0], 0);
		((ShaderProgramOpenGL*)Bloom)->SetUniform(BloomFinalPassVerticalBlur, (TextureOpenGL*)BloomVerticalBlurPass.ColorTextures[0], 1);
		((ShaderProgramOpenGL*)Bloom)->SetUniform(BloomFinalPassIntensity, BloomIntensity);
		Quad.Render();

		BloomFinalPass.Unbind();
	}

	void Renderer::RenderIcons()
	{
		auto Icon = gDevice->GetDefaultShaders()->Icon;

		static int IconTextureID = ((ShaderProgramOpenGL*)(Icon))->GetFastUniform("Texture");
		static int IconPosID = ((ShaderProgramOpenGL*)(Icon))->GetFastUniform("Pos");
		static int IconSizeID = ((ShaderProgramOpenGL*)(Icon))->GetFastUniform("Size");

		auto DrawIcon = [&](Vector4 Coords)
		{
			Coords = MainCamera.GetViewProjection() * Coords;

			if (Coords.W > 0.0f)
			{
				Coords /= Coords.W;

				float Aspect = (float)ContextSize.X / (float)ContextSize.Y;


				((ShaderProgramOpenGL*)(Icon))->SetUniform(IconPosID, Coords.XY());
				((ShaderProgramOpenGL*)(Icon))->SetUniform(IconSizeID, 0.1f / Vector2(Aspect, 1));
				Quad.Render();
			}
		};

		((ShaderProgramOpenGL*)(Icon))->Bind();

		State.SetBlending(true);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		((ShaderProgramOpenGL*)(Icon))->SetUniform(IconTextureID, (TextureOpenGL*)EditorIconSun, 0);
		for (const auto& Elem : Scn->Lights)
		{
			if (Elem != nullptr)
				if (Elem->Type == 0)
					DrawIcon(Vector4(Elem->Pos, 1));
		}

		((ShaderProgramOpenGL*)(Icon))->SetUniform(IconTextureID, (TextureOpenGL*)EditorIconLamp, 0);
		for (const auto& Elem : Scn->Lights)
		{
			if (Elem != nullptr)
				if (Elem->Type == 1)
					DrawIcon(Vector4(Elem->Pos, 1));
		}

		((ShaderProgramOpenGL*)(Icon))->SetUniform(IconTextureID, (TextureOpenGL*)EditorIconFlashlight, 0);
		for (const auto& Elem : Scn->Lights)
		{
			if (Elem != nullptr)
				if (Elem->Type == 2)
					DrawIcon(Vector4(Elem->Pos, 1));
		}

		((ShaderProgramOpenGL*)(Icon))->SetUniform(IconTextureID, (TextureOpenGL*)EditorIconAudio, 0);
		for (const auto& Elem : Scn->Audio.Mixer.Sources)
		{
			if (Elem != nullptr)
				DrawIcon(Vector4(Elem->Position, 1));
		}

		((ShaderProgramOpenGL*)(Icon))->SetUniform(IconTextureID, (TextureOpenGL*)EditorIconParticles, 0);
		for (const auto& Elem : TransparentObjects)
		{
			if (Elem.Particles != nullptr)
				DrawIcon(Vector4(Scn->Objects[Elem.Index]->transform.Position, 1));
		}

		((ShaderProgramOpenGL*)(Icon))->Unbind();
	}

	void Renderer::Render()
	{
		auto FinalPassShader = gDevice->GetDefaultShaders()->Final;
		static int FinalBaseTextureID = ((ShaderProgramOpenGL*)(FinalPassShader))->GetFastUniform("BaseTexture");
		static int FinalGamma = ((ShaderProgramOpenGL*)FinalPassShader)->GetFastUniform("Gamma");
		static int FinalExposure = ((ShaderProgramOpenGL*)FinalPassShader)->GetFastUniform("Exposure");

		if (ContextSize.X == 0) ContextSize.X = 1;
		if (ContextSize.Y == 0) ContextSize.Y = 1;

		PolygonsRendered = 0;
		OpaqueObjectsRendered = 0;
		TransparentObjectsRendered = 0;

		CompileLists();
		SortLists();

		PROFILE_GPU(ProfileModuleGPU::GPU);

		Base.Bind({ 1, 1, 1, 0 }, {0}, ContextSize);

		RenderOpaqueStage();
		RenderSkyStage();
		RenderTransparentStage();

		Base.Unbind();

		State.Clear();

		//Base.Bind({ 1, 1, 1, 0 }, {0}, ContextSize);
		//Base.Unbind();

		//FinalPass.Bind({}, {}, ContextSize);
		//FinalPass.Unbind();

		//glBindFramebuffer(GL_READ_FRAMEBUFFER, ((FramebufferOpenGL*)BaseMSAA.FB)->ID);
		//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		//glBlitFramebuffer(0, 0, ContextSize.X, ContextSize.Y, 0, 0, ContextSize.X, ContextSize.Y, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		Texture* Final = Base.ColorTextures[0];

		if (BloomEnable)
		{
			RenderBloom();
			Final = BloomFinalPass.ColorTextures[0];
		}

		// Final stage
		{
			PROFILE_GPU(ProfileModuleGPU::FinalStage);

			FinalPass.Bind({}, {}, ContextSize);

			((ShaderProgramOpenGL*)FinalPassShader)->Bind();
			((ShaderProgramOpenGL*)FinalPassShader)->SetUniform(FinalBaseTextureID, (TextureOpenGL*)Final, 0);
			((ShaderProgramOpenGL*)FinalPassShader)->SetUniform(FinalExposure, Exposure);
			((ShaderProgramOpenGL*)FinalPassShader)->SetUniform(FinalGamma, Gamma);
			Quad.Render();
			((ShaderProgramOpenGL*)FinalPassShader)->Unbind();

			if (EditMode)
			{
				RenderIcons();
			}

			FinalPass.Unbind();
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
		return FinalPass.ColorTextures[0];
	}

	Renderer::~Renderer()
	{
		delete Blob;
		delete EditorIconLamp;
		//delete NoneShader;
		//delete BloomBrightShader;
		//delete GaussBlurShader;
		//delete BloomShader;
		//delete LensFlareShader;
	}

}


