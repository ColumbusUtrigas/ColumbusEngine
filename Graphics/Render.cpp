#include <Graphics/Render.h>
#include <Graphics/Device.h>
#include <Scene/ComponentMeshRenderer.h>
#include <Scene/ComponentParticleSystem.h>
#include <Scene/Component.h>
#include <Math/Frustum.h>
#include <GL/glew.h>
#include <algorithm>

#include <Graphics/OpenGL/ShaderOpenGL.h>

namespace Columbus
{
	Texture* BlackTexture;

	ShaderProgram* NoneShader;
	ShaderProgram* BloomBrightShader;
	ShaderProgram* GaussBlurShader;
	ShaderProgram* BloomShader;

	Skybox* gSky;

	struct
	{
		Material::Cull Culling = Material::Cull::No;
		bool DepthWriting = true;
	} State;

	struct MaterialRenderData
	{
		int32 Model;
		int32 ViewProjection;

		int32 TexturesIDs[11];

		int32 HasDiffuseMap;
		int32 HasNormalMap;
		int32 HasRoughnessMap;
		int32 HasMetallicMap;
		int32 HasOcclusionMap;
		int32 HasDetailDiffuseMap;
		int32 HasDetailNormalMap;

		int32 Tiling;
		int32 DetailTiling;
		int32 Color;
		int32 Roughness;
		int32 Metallic;
		int32 EmissionStrength;
		int32 Transparent;

		int32 CameraPosition;

		int32 Lighting;
	};

	namespace
	{
		ShaderProgram* PreviousShader;
		ShaderProgram* CurrentShader;

		Material PreviousMaterial;
		Material CurrentMaterial;

		Mesh* PreviousMesh;
		Mesh* CurrentMesh;

		void ClearOptimizations()
		{
			PreviousShader = nullptr;
			CurrentShader = nullptr;

			PreviousMaterial = Material();
			CurrentMaterial = Material();

			PreviousMesh = nullptr;
			CurrentMesh = nullptr;

			State.Culling = Material::Cull::No;
			State.DepthWriting = true;
		}
	}

	static void PrepareFaceCulling(Material::Cull Culling)
	{
		State.Culling = Culling;

		switch (Culling)
		{
		case Material::Cull::No:           glDisable(GL_CULL_FACE); break;
		case Material::Cull::Front:        glEnable(GL_CULL_FACE);  glCullFace(GL_FRONT); break;
		case Material::Cull::Back:         glEnable(GL_CULL_FACE);  glCullFace(GL_BACK); break;
		case Material::Cull::FrontAndBack: glEnable(GL_CULL_FACE);  glCullFace(GL_FRONT_AND_BACK); break;
		}
	}

	static void PrepareDepthWriting(bool DepthWriting)
	{
		State.DepthWriting = DepthWriting;

		glDepthMask(DepthWriting ? GL_TRUE : GL_FALSE);
	}

	static void ShaderSetMaterial(const Transform& Trans, const Camera& MainCamera)
	{
		static const char* const Names[11] = { "uMaterial.DiffuseMap", "uMaterial.NormalMap", "uMaterial.RoughnessMap", "uMaterial.MetallicMap", "uMaterial.OcclusionMap", "uMaterial.EmissionMap", "uMaterial.DetailDiffuseMap", "uMaterial.DetailNormalMap", "uMaterial.IrradianceMap", "uMaterial.EnvironmentMap", "uMaterial.IntegrationMap" };

		#define CheckShader() (CurrentShader != PreviousShader)
		#define CheckParameter(x) (CurrentMaterial.x != PreviousMaterial.x) || CheckShader()

		if (CurrentShader != nullptr)
		{
			Texture* Textures[11] = { CurrentMaterial.DiffuseTexture, CurrentMaterial.NormalTexture, CurrentMaterial.RoughnessTexture, CurrentMaterial.MetallicTexture, CurrentMaterial.OcclusionMap, CurrentMaterial.EmissionMap, CurrentMaterial.DetailDiffuseMap, CurrentMaterial.DetailNormalMap, gSky->GetIrradianceMap(), gSky->GetPrefilterMap(), gSky->GetIntegrationMap() };
			Texture* LastTextures[11] = { PreviousMaterial.DiffuseTexture, PreviousMaterial.NormalTexture, PreviousMaterial.RoughnessTexture, PreviousMaterial.MetallicTexture, PreviousMaterial.OcclusionMap, PreviousMaterial.EmissionMap, PreviousMaterial.DetailDiffuseMap, PreviousMaterial.DetailNormalMap, nullptr, nullptr, nullptr };

			MaterialRenderData* RenderData = (MaterialRenderData*)((ShaderProgramOpenGL*)(CurrentShader))->RenderData;
			ShaderProgramOpenGL* ShaderOGL = (ShaderProgramOpenGL*)CurrentShader;

			if (RenderData == nullptr)
			{
				MaterialRenderData* NewRenderData = new MaterialRenderData();

				for (int i = 0; i < 11; i++)
				{
					NewRenderData->TexturesIDs[i] = ShaderOGL->GetFastUniform(Names[i]);
				}

				NewRenderData->Model          = ShaderOGL->GetFastUniform("uModel");
				NewRenderData->ViewProjection = ShaderOGL->GetFastUniform("uViewProjection");

				NewRenderData->HasDiffuseMap       = ShaderOGL->GetFastUniform("uMaterial.HasDiffuseMap");
				NewRenderData->HasNormalMap        = ShaderOGL->GetFastUniform("uMaterial.HasNormalMap");
				NewRenderData->HasRoughnessMap     = ShaderOGL->GetFastUniform("uMaterial.HasRoughnessMap");
				NewRenderData->HasMetallicMap      = ShaderOGL->GetFastUniform("uMaterial.HasMetallicMap");
				NewRenderData->HasOcclusionMap     = ShaderOGL->GetFastUniform("uMaterial.HasOcclusionMap");
				NewRenderData->HasDetailDiffuseMap = ShaderOGL->GetFastUniform("uMaterial.HasDetailDiffuseMap");
				NewRenderData->HasDetailNormalMap  = ShaderOGL->GetFastUniform("uMaterial.HasDetailNormalMap");

				NewRenderData->Tiling           = ShaderOGL->GetFastUniform("uMaterial.Tiling");
				NewRenderData->DetailTiling     = ShaderOGL->GetFastUniform("uMaterial.DetailTiling");
				NewRenderData->Color            = ShaderOGL->GetFastUniform("uMaterial.Color");
				NewRenderData->Roughness        = ShaderOGL->GetFastUniform("uMaterial.Roughness");
				NewRenderData->Metallic         = ShaderOGL->GetFastUniform("uMaterial.Metallic");
				NewRenderData->EmissionStrength = ShaderOGL->GetFastUniform("uMaterial.EmissionStrength");
				NewRenderData->Transparent      = ShaderOGL->GetFastUniform("uMaterial.Transparent");

				NewRenderData->CameraPosition   = ShaderOGL->GetFastUniform("uCamera.Position");
				NewRenderData->Lighting         = ShaderOGL->GetFastUniform("uLighting");

				ShaderOGL->RenderData = NewRenderData;
				RenderData = NewRenderData;
			}

			ShaderOGL->SetUniform(RenderData->Model, false, Trans.GetMatrix());
			ShaderOGL->SetUniform(RenderData->ViewProjection, false, MainCamera.GetViewProjection());

			for (int32 i = 0; i < 11; i++)
			{
				if (Textures[i] != LastTextures[i] || CheckShader())
				{
					if (Textures[i] != nullptr)
					{
						ShaderOGL->SetUniform(RenderData->TexturesIDs[i], (TextureOpenGL*)Textures[i], i);
					}
					else
					{
						ShaderOGL->SetUniform(RenderData->TexturesIDs[i], (TextureOpenGL*)BlackTexture, i);
					}
				}
			}

			ShaderOGL->SetUniform(RenderData->HasDiffuseMap,       CurrentMaterial.DiffuseTexture   != nullptr);
			ShaderOGL->SetUniform(RenderData->HasNormalMap,        CurrentMaterial.NormalTexture    != nullptr);
			ShaderOGL->SetUniform(RenderData->HasRoughnessMap,     CurrentMaterial.RoughnessTexture != nullptr);
			ShaderOGL->SetUniform(RenderData->HasMetallicMap,      CurrentMaterial.MetallicTexture  != nullptr);
			ShaderOGL->SetUniform(RenderData->HasOcclusionMap,     CurrentMaterial.OcclusionMap     != nullptr);
			ShaderOGL->SetUniform(RenderData->HasDetailDiffuseMap, CurrentMaterial.DetailDiffuseMap != nullptr);
			ShaderOGL->SetUniform(RenderData->HasDetailNormalMap,  CurrentMaterial.DetailNormalMap  != nullptr);

			if (CheckParameter(Tiling))           ShaderOGL->SetUniform(RenderData->Tiling,           CurrentMaterial.Tiling);
			if (CheckParameter(DetailTiling))     ShaderOGL->SetUniform(RenderData->DetailTiling,     CurrentMaterial.DetailTiling);
			if (CheckParameter(Color))            ShaderOGL->SetUniform(RenderData->Color,            CurrentMaterial.Color);
			if (CheckParameter(Roughness))        ShaderOGL->SetUniform(RenderData->Roughness,        CurrentMaterial.Roughness);
			if (CheckParameter(Metallic))         ShaderOGL->SetUniform(RenderData->Metallic,         CurrentMaterial.Metallic);
			if (CheckParameter(EmissionStrength)) ShaderOGL->SetUniform(RenderData->EmissionStrength, CurrentMaterial.EmissionStrength);
			if (CheckParameter(Transparent))      ShaderOGL->SetUniform(RenderData->Transparent,      CurrentMaterial.Transparent);
			if (CheckShader())                    ShaderOGL->SetUniform(RenderData->CameraPosition,   MainCamera.Pos);
		}
	}

	static void ShaderSetLights(const std::vector<Light*>& InLights)
	{
		static constexpr int LightsCount = 4;
		static float Lights[15 * LightsCount];

		if (CurrentShader != nullptr)
		{
			for (auto& Light : InLights)
			{
				if (Light != nullptr)
				{
					uint32 Counter = 0;

					for (auto& L : InLights)
					{
						uint32 Offset = Counter * 15;

						if (InLights.size() > Counter && Counter < LightsCount)
						{
							Lights[Offset + 0] = L->Color.X;
							Lights[Offset + 1] = L->Color.Y;
							Lights[Offset + 2] = L->Color.Z;
							Lights[Offset + 3] = L->Pos.X;
							Lights[Offset + 4] = L->Pos.Y;
							Lights[Offset + 5] = L->Pos.Z;
							Lights[Offset + 6] = L->Dir.X;
							Lights[Offset + 7] = L->Dir.Y;
							Lights[Offset + 8] = L->Dir.Z;
							Lights[Offset + 9] = (float)L->Type;
							Lights[Offset + 10] = L->Constant;
							Lights[Offset + 11] = L->Linear;
							Lights[Offset + 12] = L->Quadratic;
							Lights[Offset + 13] = L->InnerCutoff;
							Lights[Offset + 14] = L->OuterCutoff;
						}

						Counter++;
					}
				}
			}

			ShaderProgramOpenGL* ShaderOGL = (ShaderProgramOpenGL*)CurrentShader;
			MaterialRenderData* RenderData = (MaterialRenderData*)ShaderOGL->RenderData;

			ShaderOGL->SetUniform(RenderData->Lighting, 15 * LightsCount * sizeof(float), Lights);
		}
	}

	Renderer::Renderer()
	{
		BlackTexture = gDevice->CreateTexture();

		uint8 Zero = 0;
		BlackTexture->Create2D(Texture::Properties(1, 1, 0, TextureFormat::R8));
		BlackTexture->Load(&Zero, Texture::Properties(1, 1, 0, TextureFormat::R8));

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
	}

	void Renderer::SetRenderList(std::map<uint32, SmartPointer<GameObject>>* List)
	{
		RenderList = List;
	}

	void Renderer::CompileLists()
	{
		gSky = Sky;

		OpaqueObjects.clear();
		TransparentObjects.clear();

		ComponentMeshRenderer* MeshRenderer;
		ComponentParticleSystem* ParticleSystem;

		ParticleEmitter* Emitter;
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
							Object.second->GetMaterial().Reflection = Sky->GetIrradianceMap();

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
					Emitter = ParticleSystem->GetEmitter();

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
		Vector3 CameraPosition = MainCamera.Pos;
		Vector3 APosition, BPosition;
		double ADistance, BDistance;

		auto OpaqueSorter = [&](const OpaqueRenderData& A, const OpaqueRenderData& B)->bool
		{
			APosition = A.ObjectTransform.GetPos();
			BPosition = B.ObjectTransform.GetPos();

			ADistance = pow(CameraPosition.X - APosition.X, 2) + pow(CameraPosition.Y - APosition.Y, 2) + pow(CameraPosition.Z - APosition.Z, 2);
			BDistance = pow(CameraPosition.X - BPosition.X, 2) + pow(CameraPosition.Y - BPosition.Y, 2) + pow(CameraPosition.Z - BPosition.Z, 2);

			return ADistance < BDistance;
		};

		auto TransparentSorter = [&](const TransparentRenderData& A, const TransparentRenderData& B)->bool
		{
			APosition = A.ObjectTransform.GetPos();
			BPosition = B.ObjectTransform.GetPos();

			ADistance = pow(CameraPosition.X - APosition.X, 2) + pow(CameraPosition.Y - APosition.Y, 2) + pow(CameraPosition.Z - APosition.Z, 2);
			BDistance = pow(CameraPosition.X - BPosition.X, 2) + pow(CameraPosition.Y - BPosition.Y, 2) + pow(CameraPosition.Z - BPosition.Z, 2);

			return ADistance > BDistance;
		};

		std::sort(OpaqueObjects.begin(), OpaqueObjects.end(), OpaqueSorter);
		std::sort(TransparentObjects.begin(), TransparentObjects.end(), TransparentSorter);
	}

	void Renderer::RenderOpaqueStage()
	{
		uint32 PolygonsRendered = 0;

		if (RenderList != nullptr && OpaqueObjects.size() != 0)
		{
			ClearOptimizations();

			glDisable(GL_CULL_FACE);
			glDisable(GL_BLEND);
			glDepthMask(GL_TRUE);

			for (auto& Object : OpaqueObjects)
			{
				CurrentShader = Object.ObjectMaterial.GetShader();
				CurrentMaterial = Object.ObjectMaterial;
				CurrentMesh = Object.Object;
				
				if (CurrentShader != nullptr)
				{
					if (CurrentShader != PreviousShader)
					{
						if (!CurrentShader->IsError())
						{
							if (!CurrentShader->IsCompiled())
							{
								CurrentShader->Compile();
							}
						}

						CurrentShader->Bind();
					}

					PrepareFaceCulling(Object.ObjectMaterial.Culling);
					PrepareDepthWriting(Object.ObjectMaterial.DepthWriting);

					ShaderSetMaterial(Object.ObjectTransform, MainCamera);
					ShaderSetLights(Object.Object->Lights);

					if (CurrentMesh != PreviousMesh)
					{
						Object.Object->Bind();
					}

					PolygonsRendered += Object.Object->Render();
				}

				PreviousShader = Object.ObjectMaterial.GetShader();
				PreviousMesh = Object.Object;
				PreviousMaterial = Object.ObjectMaterial;
				State.Culling = Object.ObjectMaterial.Culling;
				State.DepthWriting = Object.ObjectMaterial.DepthWriting;
			}

			CurrentMesh->Unbind();
		}

		if (Sky != nullptr)
		{
			Sky->Render();
		}
	}

	void Renderer::RenderTransparentStage()
	{
		if (RenderList != nullptr && TransparentObjects.size() != 0)
		{
			ClearOptimizations();
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glDepthMask(GL_FALSE);

			for (auto& Object : TransparentObjects)
			{
				if (Object.MeshObject != nullptr)
				{
					CurrentShader = Object.ObjectMaterial.GetShader();
					CurrentMaterial = Object.ObjectMaterial;
					CurrentMesh = Object.MeshObject;

					if (CurrentShader != nullptr)
					{
						if (!CurrentShader->IsError())
						{
							if (!CurrentShader->IsCompiled())
							{
								CurrentShader->Compile();
							}
						}

						CurrentShader->Bind();

						ShaderSetMaterial(Object.ObjectTransform, MainCamera);
						ShaderSetLights(Object.MeshObject->Lights);

						Object.MeshObject->Bind();

						if (Object.ObjectMaterial.Culling == Material::Cull::No)
						{
							glDepthMask(GL_TRUE);
							CurrentShader->SetUniform1i("uMaterial.Transparent", 0);
							PrepareFaceCulling(Material::Cull::No);
							CurrentMesh->Render();

							glDepthMask(GL_FALSE);
							CurrentShader->SetUniform1i("uMaterial.Transparent", 1);
							PrepareFaceCulling(Material::Cull::Front);
							CurrentMesh->Render();
							PrepareFaceCulling(Material::Cull::Back);
							CurrentMesh->Render();
						}
						else
						{
							PrepareFaceCulling(Object.ObjectMaterial.Culling);

							glDepthMask(GL_TRUE);
							CurrentShader->SetUniform1i("uMaterial.Transparent", 0);
							CurrentMesh->Render();

							glDepthMask(GL_FALSE);
							CurrentShader->SetUniform1i("uMaterial.Transparent", 1);
							CurrentMesh->Render();
						}

						Object.MeshObject->Unbind();
					}

					PreviousShader = Object.ObjectMaterial.GetShader();
					PreviousMesh = Object.MeshObject;
					PreviousMaterial = Object.ObjectMaterial;
				}

				if (Object.ParticleObject != nullptr)
				{
					PrepareFaceCulling(Material::Cull::Back);
					Object.ParticleObject->Render();
				}
			}
		}

		glDepthMask(GL_TRUE);
	}

	void Renderer::Render(Renderer::Stage RenderStage)
	{
		if (RenderList != nullptr)
		{
			switch (RenderStage)
			{
				case Renderer::Stage::Opaque:      RenderOpaqueStage();      break;
				case Renderer::Stage::Transparent: RenderTransparentStage(); break;
			}
		}

		PrepareFaceCulling(Material::Cull::No);
	}

	void Renderer::Render()
	{
		static int NoneShaderBaseTextureID = ((ShaderProgramOpenGL*)(NoneShader))->GetFastUniform("BaseTexture");

		CompileLists();
		SortLists();

		BaseEffect.Bind({ 1, 1, 1, 0 }, ContextSize);

		RenderOpaqueStage();
		RenderTransparentStage();

		BaseEffect.Unbind();

		NoneShader->Bind();
		((ShaderProgramOpenGL*)(NoneShader))->SetUniform(NoneShaderBaseTextureID, (TextureOpenGL*)BaseEffect.ColorTextures[0], 0);
		Quad.Render();
	}

	Renderer::~Renderer()
	{
		delete BlackTexture;
		delete NoneShader;
		delete BloomBrightShader;
		delete GaussBlurShader;
		delete BloomShader;
	}

}



