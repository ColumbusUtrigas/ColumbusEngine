#include <Graphics/Render.h>
#include <Graphics/Device.h>
#include <Scene/ComponentMeshRenderer.h>
#include <Scene/ComponentMeshInstancedRenderer.h>
#include <Scene/ComponentParticleSystem.h>
#include <Scene/Component.h>
#include <Math/Frustum.h>
#include <Core/Templates/Sort.h>

namespace Columbus
{

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
		}
	}

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

		if (CurrentMaterial != PreviousMaterial)
		{
			if (CurrentShader != nullptr)
			{
				if (CurrentShader->IsCompiled())
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
								glBindTexture(GL_TEXTURE_2D, 0);
							}
						}
					}

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
							Lights[Offset + 0] = L->getColor().X;
							Lights[Offset + 1] = L->getColor().Y;
							Lights[Offset + 2] = L->getColor().Z;
							Lights[Offset + 3] = L->getPos().X;
							Lights[Offset + 4] = L->getPos().Y;
							Lights[Offset + 5] = L->getPos().Z;
							Lights[Offset + 6] = L->getDir().X;
							Lights[Offset + 7] = L->getDir().Y;
							Lights[Offset + 8] = L->getDir().Z;
							Lights[Offset + 9] = (float)L->getType();
							Lights[Offset + 10] = L->getConstant();
							Lights[Offset + 11] = L->getLinear();
							Lights[Offset + 12] = L->getQuadratic();
							Lights[Offset + 13] = L->getInnerCutoff();
							Lights[Offset + 14] = L->getOuterCutoff();
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

	}

	void Renderer::SetRenderList(std::map<uint32, SmartPointer<GameObject>>* List)
	{
		RenderList = List;
	}

	void Renderer::CompileLists()
	{
		OpaqueObjects.Clear();
		TransparentObjects.Clear();

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
						if (ViewFrustum.Check(Mesh->GetBoundingBox() * Object.second->GetTransform().GetScale() + Object.second->GetTransform().GetPos()))
						{
							if (Object.second->GetMaterial().Transparent)
							{
								TransparentObjects.Add(TransparentRenderData(Mesh, nullptr, Object.second->GetTransform(), Object.second->GetMaterial()));
							}
							else
							{
								OpaqueObjects.Add(OpaqueRenderData(Mesh, Object.second->GetTransform(), Object.second->GetMaterial()));
							}
						}
					}
				}

				if (ParticleSystem != nullptr)
				{
					Emitter = ParticleSystem->GetEmitter();

					if (Emitter != nullptr)
					{
						//Transform Trans(Emitter->GetParticleEffect()->getPos());
						TransparentObjects.Add(TransparentRenderData(nullptr, Emitter, Object.second->GetTransform(), Object.second->GetMaterial()));
					}
				}
			}
		}

		Vector3 CameraPosition = MainCamera.getPos();
		Vector3 APosition, BPosition;
		double ADistance, BDistance;

		auto OpaqueSorter = [&](const OpaqueRenderData& A, const OpaqueRenderData& B)->bool
		{
			return A.ObjectMaterial == B.ObjectMaterial;
		};

		auto TransparentSorter = [&](const TransparentRenderData& A, const TransparentRenderData& B)->bool
		{
			APosition = A.ObjectTransform.GetPos();
			BPosition = B.ObjectTransform.GetPos();

			ADistance = pow(CameraPosition.X - APosition.X, 2) + pow(CameraPosition.Y - APosition.Y, 2) + pow(CameraPosition.Z - APosition.Z, 2);
			BDistance = pow(CameraPosition.X - BPosition.X, 2) + pow(CameraPosition.Y - BPosition.Y, 2) + pow(CameraPosition.Z - BPosition.Z, 2);

			return ADistance > BDistance;
		};

		std::sort(OpaqueObjects.GetData(), OpaqueObjects.GetData() + OpaqueObjects.GetCount(), OpaqueSorter);
		std::sort(TransparentObjects.GetData(), TransparentObjects.GetData() + TransparentObjects.GetCount(), TransparentSorter);
		//Sort(OpaqueObjects.GetData(), OpaqueObjects.GetCount(), OpaqueSorter);
		//Sort(TransparentObjects.GetData(), TransparentObjects.GetCount(), TransparentSorter);
		//std::sort(OpaqueObjects.begin(), OpaqueObjects.end(), OpaqueSorter);
		//std::sort(TransparentObjects.begin(), TransparentObjects.end(), TransparentSorter);
	}

	void Renderer::RenderOpaqueStage()
	{
		uint32 PolygonsRendered = 0;

		if (RenderList != nullptr && OpaqueObjects.GetCount() != 0)
		{
			ClearOptimizations();

			Material::Cull Culling = Material::Cull::No;
			bool DepthWriting = true;

			PrepareFaceCulling(Material::Cull::No);
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
						if (!CurrentShader->IsCompiled())
						{
							CurrentShader->Compile();
						}

						CurrentShader->Bind();
					}

					if (Culling != Object.ObjectMaterial.Culling)
					{
						PrepareFaceCulling(Object.ObjectMaterial.Culling);
					}

					if (DepthWriting != Object.ObjectMaterial.DepthWriting)
					{
						glDepthMask(Object.ObjectMaterial.DepthWriting ? GL_TRUE : GL_FALSE);
					}

					ShaderSetMatrices(CurrentShader, Object.ObjectTransform, MainCamera);
					ShaderSetLights(CurrentShader, Object.Object->Lights);
					ShaderSetMaterial(MainCamera);

					if (CurrentMesh != PreviousMesh)
					{
						Object.Object->Bind();
					}

					PolygonsRendered += Object.Object->Render(Object.ObjectTransform, CurrentShader);
				}

				PreviousShader = Object.ObjectMaterial.GetShader();
				PreviousMesh = Object.Object;
				PreviousMaterial = Object.ObjectMaterial;
				Culling = Object.ObjectMaterial.Culling;
				DepthWriting = Object.ObjectMaterial.DepthWriting;
			}

			CurrentMesh->Unbind();
		}
	}

	void Renderer::RenderTransparentStage()
	{
		if (RenderList != nullptr && TransparentObjects.GetCount() != 0)
		{
			ClearOptimizations();
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
							CurrentMesh->Render(Object.ObjectTransform, Object.ObjectMaterial.GetShader());

							glDepthMask(GL_FALSE);
							CurrentShader->SetUniform1i("uMaterial.Transparent", 1);
							PrepareFaceCulling(Material::Cull::Front);
							CurrentMesh->Render(Object.ObjectTransform, Object.ObjectMaterial.GetShader());
							PrepareFaceCulling(Material::Cull::Back);
							CurrentMesh->Render(Object.ObjectTransform, Object.ObjectMaterial.GetShader());
						}
						else
						{
							PrepareFaceCulling(Object.ObjectMaterial.Culling);

							glDepthMask(GL_TRUE);
							CurrentShader->SetUniform1i("uMaterial.Transparent", 0);
							CurrentMesh->Render(Object.ObjectTransform, Object.ObjectMaterial.GetShader());

							glDepthMask(GL_FALSE);
							CurrentShader->SetUniform1i("uMaterial.Transparent", 1);
							CurrentMesh->Render(Object.ObjectTransform, Object.ObjectMaterial.GetShader());
						}

						Object.MeshObject->Unbind();
					}

					PreviousShader = Object.ObjectMaterial.GetShader();
					PreviousMesh = Object.MeshObject;
					PreviousMaterial = Object.ObjectMaterial;
				}

				if (Object.ParticleObject != nullptr)
				{
					Object.ParticleObject->Render();
				}
			}
		}
	}

	void Renderer::Render(Renderer::Stage RenderStage)
	{
		if (RenderList != nullptr)
		{
			switch (RenderStage)
			{
				case Renderer::Stage::Opaque:
				{
					RenderOpaqueStage();
					break;
				}

				case Renderer::Stage::Transparent:
				{
					RenderTransparentStage();
					break;
				}
			}
		}

		PrepareFaceCulling(Material::Cull::No);
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



