#include <Graphics/Render.h>
#include <Graphics/Device.h>
#include <Graphics/RenderState.h>
#include <Scene/ComponentMeshRenderer.h>
#include <Scene/ComponentParticleSystem.h>
#include <Scene/Component.h>
#include <Math/Frustum.h>
#include <GL/glew.h>
#include <algorithm>

#include <Graphics/OpenGL/ShaderOpenGL.h>

#include <Graphics/Particles/ParticleEmitterLoader.h>

namespace Columbus
{
	Texture* BlackTexture;
	Texture* Blob;

	ShaderProgram* NoneShader;
	ShaderProgram* BloomBrightShader;
	ShaderProgram* GaussBlurShader;
	ShaderProgram* BloomShader;
	ShaderProgram* LensFlareShader;

	RenderState State;

	Renderer::Renderer()
	{
		BlackTexture = gDevice->CreateTexture();
		Blob = gDevice->CreateTexture();

		uint8 Zero = 0;
		BlackTexture->Create2D(Texture::Properties(1, 1, 0, TextureFormat::R8));
		BlackTexture->Load(&Zero, Texture::Properties(1, 1, 0, TextureFormat::R8));

		Blob->Load("Data/Textures/blob.png");

		BaseEffect.ColorTexturesEnablement[0] = true;
		BaseEffect.ColorTexturesEnablement[1] = true;
		BaseEffect.DepthTextureEnablement = true;

		BloomBrightPass.ColorTexturesEnablement[0] = true;

		BloomBlurPass.ColorTexturesEnablement[0] = true;
		BloomBlurPass.ColorTexturesEnablement[1] = true;

		BloomFinalPass.ColorTexturesEnablement[0] = true;

		NoneShader = gDevice->CreateShaderProgram();
		NoneShader->Load("Data/Shaders/PostProcessing.glsl");
		NoneShader->Compile();

		BloomBrightShader = gDevice->CreateShaderProgram();
		BloomBrightShader->Load("Data/Shaders/Bright.glsl");
		BloomBrightShader->Compile();

		GaussBlurShader = gDevice->CreateShaderProgram();
		GaussBlurShader->Load("Data/Shaders/GaussBlur.glsl");
		GaussBlurShader->Compile();

		BloomShader = gDevice->CreateShaderProgram();
		BloomShader->Load("Data/Shaders/Bloom.glsl");
		BloomShader->Compile();

		LensFlareShader = gDevice->CreateShaderProgram();
		LensFlareShader->Load("Data/Shaders/LensFlare.glsl");
		LensFlareShader->Compile();

		ParticleEmitterLoader::Load(TestParticles, "Data/Particles/Smoke.par");
	}

	void Renderer::SetContextSize(const iVector2& Size)
	{
		ContextSize = Size;
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

	void Renderer::SetRenderList(std::map<uint32, SmartPointer<GameObject>>* List)
	{
		RenderList = List;
	}

	void Renderer::SetLightsList(std::vector<Light*>* List)
	{
		LightsList = List;
	}

	void Renderer::CompileLists()
	{
		OpaqueObjects.clear();
		TransparentObjects.clear();

		ComponentMeshRenderer* MeshRenderer;
		ComponentParticleSystem* ParticleSystem;

		Mesh* Mesh;

		Frustum ViewFrustum(MainCamera.GetViewProjection());

		if (RenderList != nullptr)
		{
			for (auto& Object : *RenderList)
			{
				MeshRenderer = static_cast<ComponentMeshRenderer*>(Object.second->GetComponent(Component::Type::MeshRenderer));
				ParticleSystem = static_cast<ComponentParticleSystem*>(Object.second->GetComponent(Component::Type::ParticleSystem));

				if (MeshRenderer != nullptr)
				{
					Mesh = MeshRenderer->GetMesh();

					if (Mesh != nullptr)
					{
						if (ViewFrustum.Check(Mesh->GetBoundingBox() * Object.second->GetTransform().GetMatrix()))
						{
							Object.second->GetMaterial().ReflectionMap = Sky->GetIrradianceMap();

							if (Object.second->GetMaterial().Transparent)
							{
								TransparentObjects.push_back(TransparentRenderData(Mesh, nullptr, Object.second->GetTransform(), Object.second->GetMaterial()));
							}
							else
							{
								OpaqueObjects.push_back(OpaqueRenderData(Mesh, Object.second->GetTransform(), Object.second->GetMaterial()));
							}
						}
					}
				}

				if (ParticleSystem != nullptr)
				{
					auto Emitter = &ParticleSystem->Emitter;

					if (Emitter != nullptr)
					{
						TransparentObjects.push_back(TransparentRenderData(nullptr, Emitter, Object.second->GetTransform(), Object.second->GetMaterial()));
					}
				}
			}
		}
	}

	void Renderer::SortLists()
	{
		auto OpaqueSorter = [&](const OpaqueRenderData& A, const OpaqueRenderData& B)->bool
		{
			return MainCamera.Pos.LengthSquare(A.ObjectTransform.GetPos()) < MainCamera.Pos.LengthSquare(B.ObjectTransform.GetPos());
		};

		auto TransparentSorter = [&](const TransparentRenderData& A, const TransparentRenderData& B)->bool
		{
			return MainCamera.Pos.LengthSquare(A.ObjectTransform.GetPos()) > MainCamera.Pos.LengthSquare(B.ObjectTransform.GetPos());
		};

		std::sort(OpaqueObjects.begin(), OpaqueObjects.end(), OpaqueSorter);
		std::sort(TransparentObjects.begin(), TransparentObjects.end(), TransparentSorter);
	}

	void Renderer::RenderOpaqueStage()
	{
		uint32 PolygonsRendered = 0;

		State.Clear();

		glDisable(GL_BLEND);

		for (auto& Object : OpaqueObjects)
		{
			ShaderProgram* CurrentShader = Object.ObjectMaterial.GetShader();
				
			if (CurrentShader != nullptr)
			{
				State.SetCulling(Object.ObjectMaterial.Culling);
				State.SetDepthTesting(Object.ObjectMaterial.DepthTesting);
				State.SetDepthWriting(Object.ObjectMaterial.DepthWriting);
				State.SetShaderProgram(Object.ObjectMaterial.GetShader());
				State.SetMaterial(Object.ObjectMaterial, Object.ObjectTransform.GetMatrix(), Sky);
				State.SetLights(Object.Object->Lights);
				State.SetMesh(Object.Object);

				PolygonsRendered += Object.Object->Render();
			}
		}
	}

	void Renderer::RenderSkyStage()
	{
		if (Sky != nullptr)
		{
			Sky->Render();
		}
	}

	void Renderer::RenderTransparentStage()
	{
		if (RenderList != nullptr && TransparentObjects.size() != 0)
		{
			State.Clear();
			glEnable(GL_BLEND);
			glBlendEquation(GL_ADD);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			for (auto& Object : TransparentObjects)
			{
				if (Object.MeshObject != nullptr)
				{
					ShaderProgram* CurrentShader = Object.ObjectMaterial.GetShader();
					Mesh* CurrentMesh = Object.MeshObject;

					if (CurrentShader != nullptr)
					{
						State.SetDepthTesting(Object.ObjectMaterial.DepthTesting);
						State.SetShaderProgram(Object.ObjectMaterial.GetShader());
						State.SetMaterial(Object.ObjectMaterial, Object.ObjectTransform.GetMatrix(), Sky);
						State.SetLights(Object.MeshObject->Lights);
						CurrentMesh->Bind();

						if (Object.ObjectMaterial.Culling == Material::Cull::No)
						{
							State.SetDepthWriting(true);
							CurrentShader->SetUniform1i("uMaterial.Transparent", 0);
							State.SetCulling(Material::Cull::No);
							CurrentMesh->Render();

							State.SetDepthWriting(false);
							CurrentShader->SetUniform1i("uMaterial.Transparent", 1);
							State.SetCulling(Material::Cull::Front);
							CurrentMesh->Render();
							State.SetCulling(Material::Cull::Back);
							CurrentMesh->Render();
						}
						else
						{
							State.SetCulling(Object.ObjectMaterial.Culling);

							State.SetDepthWriting(true);
							CurrentShader->SetUniform1i("uMaterial.Transparent", 0);
							CurrentMesh->Render();

							State.SetDepthWriting(false);
							CurrentShader->SetUniform1i("uMaterial.Transparent", 1);
							CurrentMesh->Render();
						}

						CurrentMesh->Unbind();
					}
				}

				if (Object.Particles != nullptr)
				{
					ParticlesRender.Render(*Object.Particles, MainCamera, Object.ObjectMaterial);
					//State.SetShaderProgram(Object.ParticleObject->GetParticleEffect()->Material.GetShader());
					//State.SetCulling(Material::Cull::Back);
					//Object.ParticleObject->Render();
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

	void Renderer::Render()
	{
		static int NoneShaderBaseTextureID = ((ShaderProgramOpenGL*)(NoneShader))->GetFastUniform("BaseTexture");

		CompileLists();
		SortLists();

		BaseEffect.Bind({ 1, 1, 1, 0 }, ContextSize);

		RenderOpaqueStage();
		RenderSkyStage();
		RenderTransparentStage();

		BaseEffect.Unbind();

		NoneShader->Bind();
		((ShaderProgramOpenGL*)(NoneShader))->SetUniform(NoneShaderBaseTextureID, (TextureOpenGL*)BaseEffect.ColorTextures[0], 0);
		Quad.Render();

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

	Renderer::~Renderer()
	{
		delete BlackTexture;
		delete NoneShader;
		delete BloomBrightShader;
		delete GaussBlurShader;
		delete BloomShader;
		delete LensFlareShader;
	}

}


