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

	RenderState State;

	Renderer::Renderer()
	{
		Blob = gDevice->CreateTexture();
		Blob->Load("Data/Textures/blob.png");

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

		Post1.ColorTexturesFormats[0] = TextureFormat::R11G11B10F;
		Post1.ColorTexturesFormats[1] = TextureFormat::R11G11B10F;
		Post2.ColorTexturesFormats[0] = TextureFormat::R11G11B10F;
		Post2.ColorTexturesFormats[1] = TextureFormat::R11G11B10F;

		Final.ColorTexturesEnablement[0] = true;

		Eyes[0].ColorTexturesEnablement[0] = true;
		Eyes[0].ColorTexturesFormats[0] = TextureFormat::RGBA32F;

		Eyes[1].ColorTexturesEnablement[0] = true;
		Eyes[1].ColorTexturesFormats[0] = TextureFormat::RGBA32F;

		Eyes[0].Bind({0}, {0}, {1});
		Eyes[1].Bind({0}, {0}, {1});
		Eyes[0].Unbind();
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
								Material& Mat = Scn->Materials[Object->materialID];
								Mat.ReflectionMap = Sky->GetIrradianceMap();

								if (Mat.Transparent)
								{
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

	void Renderer::RenderOpaque()
	{
		PROFILE_GPU(ProfileModuleGPU::OpaqueStage);

		State.Clear();

		State.SetBlending(false);

		for (auto& Object : OpaqueObjects)
		{
			SmartPointer<GameObject>& GO = Scn->Objects[Object.Index];
			Material& Mat = Scn->Materials[GO->materialID];
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

	void Renderer::RenderSky()
	{
		PROFILE_GPU(ProfileModuleGPU::SkyStage);

		if (Sky != nullptr)
		{
			Sky->Render();
		}
	}

	void Renderer::RenderTransparent()
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
				Material& Mat = Scn->Materials[GO->materialID];

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

		// Bloom bright pass
		Post1.ColorTexturesEnablement[0] = true;
		Post1.Bind({}, {0}, ContextSize);

		((ShaderProgramOpenGL*)BloomBrightShader)->Bind();
		((ShaderProgramOpenGL*)BloomBrightShader)->SetUniform(BrightShaderTexture, (TextureOpenGL*)Base.ColorTextures[0], 0);
		((ShaderProgramOpenGL*)BloomBrightShader)->SetUniform(BrightShaderTreshold, BloomTreshold);
		Quad.Render();

		iVector2 Resolution;

		switch (BloomResolution)
		{
		case BloomResolutionType::Quad: Resolution = ContextSize / 2; break;
		case BloomResolutionType::Half: Resolution = iVector2(Vector2(ContextSize) / sqrtf(2.0)); break;
		case BloomResolutionType::Full: Resolution = ContextSize; break;
		}

		if (Resolution.X == 0) Resolution.X = 1;
		if (Resolution.Y == 0) Resolution.Y = 1;

		// Blur pass
		((ShaderProgramOpenGL*)Blur)->Bind();
		((ShaderProgramOpenGL*)Blur)->SetUniform(BloomBlurRadius, BloomRadius);

		for (int i = 0; i < BloomIterations; i++)
		{			
			Post2.ColorTexturesEnablement[0] = true;
			Post2.Bind({}, { 0 }, Resolution);

			((ShaderProgramOpenGL*)Blur)->SetUniform(BloomBlurTexture, (TextureOpenGL*)Post1.ColorTextures[0], 0);
			((ShaderProgramOpenGL*)Blur)->SetUniform(BloomBlurHorizontal, 1);
			Quad.Render();

			Post1.ColorTexturesEnablement[0] = true;
			Post1.Bind({}, { 0 }, Resolution);

			((ShaderProgramOpenGL*)Blur)->SetUniform(BloomBlurTexture, (TextureOpenGL*)Post2.ColorTextures[0], 0);
			((ShaderProgramOpenGL*)Blur)->SetUniform(BloomBlurHorizontal, 0);
			Quad.Render();
		}

		// Bloom final
		Post2.ColorTexturesEnablement[0] = true;
		Post2.Bind({}, {}, ContextSize);

		((ShaderProgramOpenGL*)Bloom)->Bind();
		((ShaderProgramOpenGL*)Bloom)->SetUniform(BloomFinalPassBaseTexture, (TextureOpenGL*)Base.ColorTextures[0], 0);
		((ShaderProgramOpenGL*)Bloom)->SetUniform(BloomFinalPassVerticalBlur, (TextureOpenGL*)Post1.ColorTextures[0], 1);
		((ShaderProgramOpenGL*)Bloom)->SetUniform(BloomFinalPassIntensity, BloomIntensity);
		Quad.Render();
	}

	void Renderer::RenderIcons()
	{
		auto Icon = gDevice->GetDefaultShaders()->Icon;

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

		State.SetBlending(true);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		((ShaderProgramOpenGL*)(Icon))->SetUniform(IconTextureID, (TextureOpenGL*)gDevice->GetDefaultTextures()->IconSun, 0);
		for (const auto& Elem : Scn->Lights)
		{
			if (Elem != nullptr)
				if (Elem->Type == Light::Directional)
					DrawIcon(Vector4(Elem->Pos, 1));
		}

		((ShaderProgramOpenGL*)(Icon))->SetUniform(IconTextureID, (TextureOpenGL*)gDevice->GetDefaultTextures()->IconLamp, 0);
		for (const auto& Elem : Scn->Lights)
		{
			if (Elem != nullptr)
				if (Elem->Type == Light::Point)
					DrawIcon(Vector4(Elem->Pos, 1));
		}

		((ShaderProgramOpenGL*)(Icon))->SetUniform(IconTextureID, (TextureOpenGL*)gDevice->GetDefaultTextures()->IconFlashlight, 0);
		for (const auto& Elem : Scn->Lights)
		{
			if (Elem != nullptr)
				if (Elem->Type == Light::Spot)
					DrawIcon(Vector4(Elem->Pos, 1));
		}

		((ShaderProgramOpenGL*)(Icon))->SetUniform(IconTextureID, (TextureOpenGL*)gDevice->GetDefaultTextures()->IconAudio, 0);
		for (const auto& Elem : Scn->Audio.Mixer.Sources)
		{
			if (Elem != nullptr)
				DrawIcon(Vector4(Elem->Position, 1));
		}

		((ShaderProgramOpenGL*)(Icon))->SetUniform(IconTextureID, (TextureOpenGL*)gDevice->GetDefaultTextures()->IconParticles, 0);
		for (const auto& Elem : TransparentObjects)
		{
			if (Elem.Particles != nullptr)
				DrawIcon(Vector4(Scn->Objects[Elem.Index]->transform.Position, 1));
		}

		((ShaderProgramOpenGL*)(Icon))->Unbind();
	}

	void Renderer::Render()
	{
		auto ScreenSpaceShader = (ShaderProgramOpenGL*)gDevice->GetDefaultShaders()->ScreenSpace;
		auto TonemapShader = (ShaderProgramOpenGL*)gDevice->GetDefaultShaders()->Tonemap;
		auto MSAAShader = (ShaderProgramOpenGL*)gDevice->GetDefaultShaders()->ResolveMSAA;
		auto FXAAShader = (ShaderProgramOpenGL*)gDevice->GetDefaultShaders()->FXAA;
		auto VignetteShader = (ShaderProgramOpenGL*)gDevice->GetDefaultShaders()->Vignette;

		static int ScreenSpaceTexture = ScreenSpaceShader->GetFastUniform("BaseTexture");

		static int TonemapBaseTexture = TonemapShader->GetFastUniform("BaseTexture");
		static int TonemapGamma = TonemapShader->GetFastUniform("Gamma");
		static int TonemapExposure = TonemapShader->GetFastUniform("Exposure");
		static int TonemapType = TonemapShader->GetFastUniform("Type");

		static int MSAABaseTexture = MSAAShader->GetFastUniform("BaseTexture");
		static int MSAASamples = MSAAShader->GetFastUniform("Samples");

		static int FXAABaseTexture = FXAAShader->GetFastUniform("BaseTexture");
		static int FXAAResolution  = FXAAShader->GetFastUniform("Resolution");

		static int VignetteColorID = VignetteShader->GetFastUniform("Color");
		static int VignetteCenterID = VignetteShader->GetFastUniform("Center");
		static int VignetteIntensityID = VignetteShader->GetFastUniform("Intensity");
		static int VignetteSmoothnessID = VignetteShader->GetFastUniform("Smoothness");
		static int VignetteRadiusID = VignetteShader->GetFastUniform("Radius");

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

		GLuint BuffersAll[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		GLuint BuffersFirst[] = { GL_COLOR_ATTACHMENT0 };

		// RENDERING
		//
		//
		// Render to the MSAA buffer or not
		if (IsMSAA)
			BaseMSAA.Bind({0}, {0}, ContextSize);
		else
			Base.Bind({0}, {0}, ContextSize);

		RenderOpaque();
		glDrawBuffers(1, BuffersFirst);
		RenderSky();
		glDrawBuffers(2, BuffersAll);
		RenderTransparent();
		//
		//
		// RENDERING

		if (IsMSAA)
		{
			Base.Bind({0}, {0}, ContextSize);

			// Resolve RT0 (HDR Color)
			MSAAShader->Bind();
			MSAAShader->SetUniform(MSAABaseTexture, (TextureOpenGL*)BaseMSAA.ColorTextures[0], 0);
			MSAAShader->SetUniform(MSAASamples, (int)BaseMSAA.Multisampling);
			Quad.Render();
			MSAAShader->Unbind();

			// Resolve RT1 (Normals) and Depth
			glBindFramebuffer(GL_READ_FRAMEBUFFER, ((FramebufferOpenGL*)BaseMSAA.FB)->ID);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ((FramebufferOpenGL*)Base.FB)->ID);

			glReadBuffer(GL_COLOR_ATTACHMENT1);
			glDrawBuffer(GL_COLOR_ATTACHMENT1);
			glBlitFramebuffer(0, 0, ContextSize.X, ContextSize.Y, 0, 0, ContextSize.X, ContextSize.Y, GL_COLOR_BUFFER_BIT, GL_NEAREST);

			glBlitFramebuffer(0, 0, ContextSize.X, ContextSize.Y, 0, 0, ContextSize.X, ContextSize.Y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		}

		Base.ColorTexturesMipmaps[0] = true;
		Base.Mipmaps();

		State.SetDepthWriting(false);
		State.SetCulling(Material::Cull::No);

		if (EyeAdaptationEnable)
		{
			Eyes[CurrentEye].Bind({0}, {0}, {1});

			int Count = floor(log2(Math::Max(ContextSize.X, ContextSize.Y)));

			Base.ColorTextures[0]->SetMipmapLevel(Count);
			ScreenSpaceShader->Bind();
			ScreenSpaceShader->SetUniform(ScreenSpaceTexture, (TextureOpenGL*)Base.ColorTextures[0], 0);
			Quad.Render();
			Base.ColorTextures[0]->SetMipmapLevel(0);

			CurrentEye = CurrentEye == 0 ? 1 : 0;
		}

		Texture* FinalTex = Base.ColorTextures[0];

		if (BloomEnable)
		{
			RenderBloom();
			FinalTex = Post2.ColorTextures[0];
		}

		// Final stage
		{
			PROFILE_GPU(ProfileModuleGPU::FinalStage);

			Final.Bind({}, {}, ContextSize);

			static float E = 1.0f;

			if (EyeAdaptationEnable)
			{
				int PrevEye = CurrentEye == 0 ? 1 : 0;

				Vector4 Curr;
				float CurrLuma;

				((TextureOpenGL*)Eyes[PrevEye].ColorTextures[0])->Bind();
				glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, (void*)&Curr);

				CurrLuma = Vector3::Dot(Curr.XYZ(), Vector3(0.2125, 0.7154, 0.0721));
				float Adapted = 0.5f / Math::Clamp(CurrLuma, EyeAdaptationMin, EyeAdaptationMax);

				if (Adapted >= E)
					E += (Adapted - E) * DeltaTime * EyeAdaptationSpeedUp;
				else
					E += (Adapted - E) * DeltaTime * EyeAdaptationSpeedDown;
			} else
			{
				E = 1.0f;
			}

			TonemapShader->Bind();
			TonemapShader->SetUniform(TonemapType, (int)Tonemapping);
			TonemapShader->SetUniform(TonemapBaseTexture, (TextureOpenGL*)FinalTex, 0);
			TonemapShader->SetUniform(TonemapExposure, Exposure * E);
			TonemapShader->SetUniform(TonemapGamma, Gamma);
			Quad.Render();
			TonemapShader->Unbind();

			if (IsFXAA)
			{
				Post1.ColorTexturesEnablement[0] = true;
				Post1.Bind({}, {}, ContextSize);

				FXAAShader->Bind();
				FXAAShader->SetUniform(FXAABaseTexture, (TextureOpenGL*)Final.ColorTextures[0], 0);
				FXAAShader->SetUniform(FXAAResolution, (Vector2)ContextSize);
				Quad.Render();

				Final.FB->Bind();

				ScreenSpaceShader->Bind();
				ScreenSpaceShader->SetUniform(ScreenSpaceTexture, (TextureOpenGL*)Post1.ColorTextures[0], 0);
				Quad.Render();
			}

			if (DrawIcons)
			{
				RenderIcons();
			}

			if (VignetteEnable)
			{
				State.SetBlending(true);
				VignetteShader->Bind();
				VignetteShader->SetUniform(VignetteColorID, VignetteColor);
				VignetteShader->SetUniform(VignetteCenterID, VignetteCenter);
				VignetteShader->SetUniform(VignetteIntensityID, VignetteIntensity);
				VignetteShader->SetUniform(VignetteSmoothnessID, VignetteSmoothness);
				VignetteShader->SetUniform(VignetteRadiusID, VignetteRadius);
				Quad.Render();
				VignetteShader->Unbind();
			}

			Final.Unbind();
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


