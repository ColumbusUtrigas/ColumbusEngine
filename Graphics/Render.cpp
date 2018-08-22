#include <Graphics/Render.h>
#include <Graphics/Device.h>
#include <Scene/ComponentMeshRenderer.h>
#include <Scene/ComponentMeshInstancedRenderer.h>
#include <Scene/ComponentParticleSystem.h>
#include <Scene/Component.h>

namespace Columbus
{

	static void PrepareFaceCulling(Material::Cull Culling)
	{
		switch (Culling)
		{
			case Material::Cull::No:
			{
				glDisable(GL_CULL_FACE);
				break;
			}

			case Material::Cull::Front:
			{
				glEnable(GL_CULL_FACE);
				glCullFace(GL_FRONT);
				break;
			}

			case Material::Cull::Back:
			{
				glEnable(GL_CULL_FACE);
				glCullFace(GL_BACK);
				break;
			}

			case Material::Cull::FrontAndBack:
			{
				glEnable(GL_CULL_FACE);
				glCullFace(GL_FRONT_AND_BACK);
				break;
			}
		}
	}

	static void ShaderSetMatrices(ShaderProgram* Program, Transform* InTransform, Camera* InCamera)
	{
		static float sModelMatrix[16];
		static float sViewMatrix[16];
		static float sProjectionMatrix[16];

		if (Program != nullptr && InTransform != nullptr && InCamera != nullptr)
		{
			if (Program->IsCompiled())
			{
				InTransform->GetMatrix().ElementsTransposed(sModelMatrix);
				InCamera->getViewMatrix().Elements(sViewMatrix);
				InCamera->getProjectionMatrix().ElementsTransposed(sProjectionMatrix);

				Program->SetUniformMatrix("uModel", sModelMatrix);
				Program->SetUniformMatrix("uView", sViewMatrix);
				Program->SetUniformMatrix("uProjection", sProjectionMatrix);
			}
		}
	}

	static void ShaderSetMaterial(ShaderProgram* Program, Material* InMaterial)
	{
		if (Program != nullptr && InMaterial != nullptr)
		{
			if (Program->IsCompiled())
			{
				Texture* Textures[7] = { InMaterial->DiffuseTexture, InMaterial->SpecularTexture, InMaterial->NormalTexture, InMaterial->DetailDiffuseMap, InMaterial->DetailNormalMap, InMaterial->Reflection, InMaterial->EmissionMap };
				static std::string Names[7] = { "uMaterial.DiffuseMap" , "uMaterial.SpecularMap", "uMaterial.NormalMap", "uMaterial.DetailDiffuseMap", "uMaterial.DetailNormalMap", "uMaterial.ReflectionMap", "uMaterial.EmissionMap" };

				for (int32 i = 0; i < 7; i++)
				{
					if (Textures[i] != nullptr)
					{
						if (i == 7) printf("a\n");
						glActiveTexture(GL_TEXTURE0 + i);
						Program->SetUniform1i(Names[i], i);
						Textures[i]->bind();
					}
					else
					{
						glActiveTexture(GL_TEXTURE0 + i);
						Program->SetUniform1i(Names[i], i);
						glBindTexture(GL_TEXTURE_2D, 0);
					}
				}

				Program->SetUniform2f("uMaterial.Tiling", InMaterial->Tiling);
				Program->SetUniform2f("uMaterial.DetailTiling", InMaterial->DetailTiling);
				Program->SetUniform4f("uMaterial.Color", InMaterial->Color);
				Program->SetUniform3f("uMaterial.AmbientColor", InMaterial->AmbientColor);
				Program->SetUniform3f("uMaterial.DiffuseColor", InMaterial->DiffuseColor);
				Program->SetUniform3f("uMaterial.SpecularColor", InMaterial->SpecularColor);
				Program->SetUniform1f("uMaterial.ReflectionPower", InMaterial->ReflectionPower);
				Program->SetUniform1f("uMaterial.EmissionStrength", InMaterial->EmissionStrength);
				Program->SetUniform1f("uMaterial.DetailNormalStrength", InMaterial->DetailNormalStrength);
				Program->SetUniform1f("uMaterial.Rim", InMaterial->Rim);
				Program->SetUniform1f("uMaterial.RimPower", InMaterial->RimPower);
				Program->SetUniform1f("uMaterial.RimBias", InMaterial->RimBias);
				Program->SetUniform3f("uMaterial.RimColor", InMaterial->RimColor);
				Program->SetUniform1i("uMaterial.Lighting", InMaterial->getLighting());
			}
		}
	}

	Renderer::Renderer()
	{

	}

	void Renderer::SetRenderList(std::map<uint32, SmartPointer<GameObject>>* List)
	{
		RenderList = List;
	}

	void Renderer::CompileLists()
	{
		Meshes.clear();
		MeshesInstanced.clear();
		ParticleEmitters.clear();

		ComponentMeshRenderer* MeshRenderer;
		ComponentMeshInstancedRenderer* MeshInstancedRenderer;
		ComponentParticleSystem* ParticleSystem;

		ParticleEmitter* Emitter;
		Mesh* Mesh;

		if (RenderList != nullptr)
		{
			for (auto& Object : *RenderList)
			{
				MeshRenderer = static_cast<ComponentMeshRenderer*>(Object.second->GetComponent(Component::Type::MeshRenderer));
				MeshInstancedRenderer = static_cast<ComponentMeshInstancedRenderer*>(Object.second->GetComponent(Component::Type::MeshInstancedRenderer));
				ParticleSystem = static_cast<ComponentParticleSystem*>(Object.second->GetComponent(Component::Type::ParticleSystem));

				if (MeshRenderer != nullptr)
				{
					Mesh = MeshRenderer->GetMesh();

					if (Mesh != nullptr)
					{
						Meshes.push_back(MeshRenderData(Mesh, Object.second->GetTransform(), Object.second->GetMaterial()));
					}
				}

				if (MeshInstancedRenderer != nullptr)
				{

				}

				if (ParticleSystem != nullptr)
				{
					Emitter = ParticleSystem->GetEmitter();

					if (Emitter != nullptr)
					{
						ParticleEmitters.push_back(Emitter);
					}
				}
			}
		}

		Vector3 CameraPosition = MainCamera.getPos();
		Vector3 APosition, BPosition;

		auto MaterialSorter = [&](MeshRenderData A, MeshRenderData B)->bool
		{
			return A.ObjectMaterial == B.ObjectMaterial;
		};

		auto ParticleSorter = [&](ParticleEmitter* A, ParticleEmitter* B)->bool
		{
			double ADistance, BDistance;

			APosition = A->getParticleEffect()->getPos();
			BPosition = B->getParticleEffect()->getPos();

			ADistance = pow(CameraPosition.X - APosition.X, 2) + pow(CameraPosition.Y - APosition.Y, 2) + pow(CameraPosition.Z - APosition.Z, 2);
			BDistance = pow(CameraPosition.X - BPosition.X, 2) + pow(CameraPosition.Y - BPosition.Y, 2) + pow(CameraPosition.Z - BPosition.Z, 2);

			return ADistance > BDistance;
		};

		std::sort(Meshes.begin(), Meshes.end(), MaterialSorter);
		std::sort(ParticleEmitters.begin(), ParticleEmitters.end(), ParticleSorter);
	}

	void Renderer::Render(Renderer::Stage RenderStage)
	{
		uint32 PolygonsRendered = 0;

		if (RenderList != nullptr)
		{
			switch (RenderStage)
			{
				case Renderer::Stage::Opaque:
				{
					ShaderProgram* CurrentShader = nullptr;
					ShaderProgram* PreviousShader = nullptr;

					Material PreviousMaterial;

					Material::Cull Culling = Material::Cull::No;
					bool DepthWriting = true;

					PrepareFaceCulling(Material::Cull::No);
					glDepthMask(GL_TRUE);

					for (auto& MeshRenderer : Meshes)
					{
						CurrentShader = MeshRenderer.ObjectMaterial.GetShader();

						if (CurrentShader != nullptr)
						{
							if (CurrentShader != PreviousShader)
							{
								if (!CurrentShader->IsCompiled())
								{
									CurrentShader->Compile();
								}

								CurrentShader->Bind();
							}

							if (Culling != MeshRenderer.ObjectMaterial.Culling)
							{
								PrepareFaceCulling(MeshRenderer.ObjectMaterial.Culling);
							}

							if (DepthWriting != MeshRenderer.ObjectMaterial.DepthWriting)
							{
								glDepthMask(MeshRenderer.ObjectMaterial.DepthWriting ? GL_TRUE : GL_FALSE);
							}

							ShaderSetMatrices(CurrentShader, &MeshRenderer.ObjectTransform, &MeshRenderer.Object->GetCamera());

							if (MeshRenderer.ObjectMaterial != PreviousMaterial)
							{
								ShaderSetMaterial(CurrentShader, &MeshRenderer.ObjectMaterial);
							}

							MeshRenderer.Object->Bind();
							PolygonsRendered += MeshRenderer.Object->Render(MeshRenderer.ObjectTransform);
							MeshRenderer.Object->Unbind();
						}

						PreviousShader = MeshRenderer.ObjectMaterial.GetShader();
						PreviousMaterial = MeshRenderer.ObjectMaterial;
						Culling = MeshRenderer.ObjectMaterial.Culling;
						DepthWriting = MeshRenderer.ObjectMaterial.DepthWriting;
					}

					for (auto& MeshInstancedRenderer : MeshesInstanced)
					{
						MeshInstancedRenderer->Render();
					}

					break;
				}

				case Renderer::Stage::Transparent:
				{
					for (auto& Emitter : ParticleEmitters)
					{
						Emitter->draw();
					}

					break;
				}
			}
		}
	}

	void Renderer::Render(std::map<uint32, SmartPointer<GameObject>>* RenderList)
	{
		if (RenderList != nullptr)
		{
			for (auto& Object : *RenderList)
			{
				ComponentMeshRenderer* MeshRenderer = static_cast<ComponentMeshRenderer*>(Object.second->GetComponent(Component::Type::MeshRenderer));

				if (MeshRenderer != nullptr)
				{
					MeshRenderer->Render(Object.second->GetTransform());
				}
			}
		}
	}

	Renderer::~Renderer()
	{

	}

}



