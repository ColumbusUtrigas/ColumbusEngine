#include <Graphics/Render.h>
#include <Graphics/Device.h>
#include <Scene/ComponentMeshRenderer.h>
#include <Scene/ComponentParticleSystem.h>
#include <Scene/Component.h>
#include <Math/Frustum.h>
#include <GL/glew.h>

namespace Columbus
{
	Texture* IrradianceMap;
	Texture* BlackTexture;

	ShaderProgram* NoneShader;
	ShaderProgram* BloomBrightShader;
	ShaderProgram* GaussBlurShader;
	ShaderProgram* BloomShader;

	struct
	{
		Material::Cull Culling = Material::Cull::No;
		bool DepthWriting = true;
	} State;

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

	static void ShaderSetMatrices(ShaderProgram* Program, const Transform& InTransform, const Camera& InCamera)
	{
		static float sModelMatrix[16];
		static float sViewProjection[16];

		if (Program != nullptr)
		{
			if (Program->IsCompiled())
			{
				InTransform.GetMatrix().Elements(sModelMatrix);
				InCamera.GetViewProjection().Elements(sViewProjection);

				Program->SetUniformMatrix("uModel", sModelMatrix);
				Program->SetUniformMatrix("uViewProjection", sViewProjection);
			}
		}
	}

	static void ShaderSetMaterial(const Camera& MainCamera)
	{
		static std::string const Names[9] = { "uMaterial.DiffuseMap", "uMaterial.NormalMap", "uMaterial.RoughnessMap", "uMaterial.MetallicMap", "uMaterial.OcclusionMap", "uMaterial.EmissionMap", "uMaterial.DetailDiffuseMap", "uMaterial.DetailNormalMap", "uMaterial.ReflectionMap"};

		#define CheckShader() (CurrentShader != PreviousShader)
		#define CheckParameter(x) (CurrentMaterial.x != PreviousMaterial.x) || CheckShader()

		if (CurrentShader != nullptr)
		{
			Texture* Textures[9] = { CurrentMaterial.DiffuseTexture, CurrentMaterial.NormalTexture, CurrentMaterial.RoughnessTexture, CurrentMaterial.MetallicTexture, CurrentMaterial.OcclusionMap, CurrentMaterial.EmissionMap, CurrentMaterial.DetailDiffuseMap, CurrentMaterial.DetailNormalMap, CurrentMaterial.Reflection };
			Texture* LastTextures[9] = { PreviousMaterial.DiffuseTexture, PreviousMaterial.NormalTexture, PreviousMaterial.RoughnessTexture, PreviousMaterial.MetallicTexture, PreviousMaterial.OcclusionMap, PreviousMaterial.EmissionMap, PreviousMaterial.DetailDiffuseMap, PreviousMaterial.DetailNormalMap, PreviousMaterial.Reflection };

			for (int32 i = 0; i < 9; i++)
			{
				if (Textures[i] != LastTextures[i] || CheckShader())
				{
					if (Textures[i] != nullptr)
					{
						glActiveTexture(GL_TEXTURE0 + i);
						CurrentShader->SetUniform1i(Names[i], i);
						Textures[i]->bind();
					}
					else
					{
						glActiveTexture(GL_TEXTURE0 + i);
						CurrentShader->SetUniform1i(Names[i], i);
						BlackTexture->bind();
					}
				}
			}

			CurrentShader->SetUniform1i("uMaterial.HasDiffuseMap",       CurrentMaterial.DiffuseTexture   != nullptr);
			CurrentShader->SetUniform1i("uMaterial.HasNormalMap",        CurrentMaterial.NormalTexture    != nullptr);
			CurrentShader->SetUniform1i("uMaterial.HasRoughnessMap",     CurrentMaterial.RoughnessTexture != nullptr);
			CurrentShader->SetUniform1i("uMaterial.HasMetallicMap",      CurrentMaterial.MetallicTexture  != nullptr);
			CurrentShader->SetUniform1i("uMaterial.HasOcclusionMap",     CurrentMaterial.OcclusionMap     != nullptr);
			CurrentShader->SetUniform1i("uMaterial.HasDetailDiffuseMap", CurrentMaterial.DetailDiffuseMap != nullptr);
			CurrentShader->SetUniform1i("uMaterial.HasDetailNormalMap",  CurrentMaterial.DetailNormalMap  != nullptr);

			if (CheckParameter(Tiling))           CurrentShader->SetUniform2f("uMaterial.Tiling", CurrentMaterial.Tiling);
			if (CheckParameter(DetailTiling))     CurrentShader->SetUniform2f("uMaterial.DetailTiling", CurrentMaterial.DetailTiling);
			if (CheckParameter(Color))            CurrentShader->SetUniform4f("uMaterial.Color", CurrentMaterial.Color);
			if (CheckParameter(Roughness))        CurrentShader->SetUniform1f("uMaterial.Roughness", CurrentMaterial.Roughness);
			if (CheckParameter(Metallic))         CurrentShader->SetUniform1f("uMaterial.Metallic", CurrentMaterial.Metallic);
			if (CheckParameter(EmissionStrength)) CurrentShader->SetUniform1f("uMaterial.EmissionStrength", CurrentMaterial.EmissionStrength);
			if (CheckParameter(Transparent))      CurrentShader->SetUniform1i("uMaterial.Transparent", CurrentMaterial.Transparent);
			if (CheckShader())                    CurrentShader->SetUniform3f("uCamera.Position", MainCamera.getPos());
		}
	}

	static void ShaderSetLights(ShaderProgram* InShader, const std::vector<Light*>& InLights)
	{
		static constexpr int LightsCount = 4;
		static float Lights[15 * LightsCount];

		if (InShader != nullptr)
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

			InShader->SetUniformArrayf("uLighting", Lights, 15 * LightsCount * sizeof(float));
		}
	}

	Renderer::Renderer()
	{
		IrradianceMap = gDevice->CreateTexture();
		BlackTexture = gDevice->CreateTexture();

		uint8 Zero = 0;
		BlackTexture->Create2D(Texture::Properties(1, 1, 0, TextureFormat::R8));
		BlackTexture->Load(&Zero, Texture::Properties(1, 1, 0, TextureFormat::R8));
		
		Image img;

		if (img.Load("Data/Skyboxes/Irradiance.dds"))
		{
			IrradianceMap->CreateCube(Texture::Properties(img.GetWidth(), img.GetHeight(), 0, img.GetFormat()));
			IrradianceMap->Load(img);
		}

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
							Object.second->GetMaterial().Reflection = IrradianceMap;

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
		Vector3 CameraPosition = MainCamera.getPos();
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

					ShaderSetMatrices(CurrentShader, Object.ObjectTransform, MainCamera);
					ShaderSetLights(CurrentShader, Object.Object->Lights);
					ShaderSetMaterial(MainCamera);

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
			Sky->draw();
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

						ShaderSetMatrices(Object.ObjectMaterial.GetShader(), Object.ObjectTransform, MainCamera);
						ShaderSetMaterial(MainCamera);
						ShaderSetLights(Object.ObjectMaterial.GetShader(), Object.MeshObject->Lights);

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
		CompileLists();
		SortLists();

		RenderOpaqueStage();
		RenderTransparentStage();

		/*BaseEffect.Bind({ 1, 1, 1, 0 }, ContextSize);

		CompileLists();
		SortLists();

		RenderOpaqueStage();
		//RenderTransparentStage();

		BaseEffect.Unbind();

		NoneShader->Bind();
		glActiveTexture(GL_TEXTURE0);
		NoneShader->SetUniform1i("BaseTexture", 0);
		BaseEffect.ColorTextures[0]->bind();
		Quad.Render();*/

		/*BloomBrightPass.Bind({ 1, 1, 1, 0 }, ContextSize);

		BloomBrightShader->Bind();
		glActiveTexture(GL_TEXTURE0);
		BloomBrightShader->SetUniform1i("BaseTexture", 0);
		BaseEffect.ColorTextures[0]->bind();
		Quad.Render();

		BloomBrightPass.Unbind();

		BloomBlurPass.Bind({ 1, 1, 1, 0 }, ContextSize);

		NoneShader->Bind();
		glActiveTexture(GL_TEXTURE0);
		NoneShader->SetUniform1i("BaseTexture", 0);
		BloomBrightPass.ColorTextures[0]->bind();
		Quad.Render();

		BloomBlurPass.Unbind();

		BloomFinalPass.Bind({ 1, 1, 1, 0 }, ContextSize);

		GaussBlurShader->Bind();
		glActiveTexture(GL_TEXTURE0);
		GaussBlurShader->SetUniform1i("BaseTexture", 0);
		BloomBlurPass.ColorTextures[0]->bind();
		GaussBlurShader->SetUniform2f("Resolution", ContextSize);

		Quad.Render();

		BloomFinalPass.Unbind();

		BloomShader->Bind();

		glActiveTexture(GL_TEXTURE0);
		BloomShader->SetUniform1i("BaseTexture", 0);
		BaseEffect.ColorTextures[0]->bind();

		glActiveTexture(GL_TEXTURE1);
		BloomShader->SetUniform1i("HorizontalBlur", 1);
		BloomFinalPass.ColorTextures[0]->bind();

		glActiveTexture(GL_TEXTURE2);
		BloomShader->SetUniform1i("VerticalBlur", 2);
		BloomFinalPass.ColorTextures[1]->bind();

		Quad.Render();*/

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	Renderer::~Renderer()
	{
		delete IrradianceMap;
		delete BlackTexture;
		delete NoneShader;
		delete BloomBrightShader;
		delete GaussBlurShader;
		delete BloomShader;
	}

}



