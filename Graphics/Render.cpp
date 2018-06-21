/************************************************
*                 Render.cpp                    *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/
#include <Graphics/Render.h>
#include <Graphics/Device.h>
#include <Scene/ComponentMeshRenderer.h>
#include <Scene/ComponentMeshInstancedRenderer.h>
#include <Scene/ComponentParticleSystem.h>
#include <Scene/Component.h>

namespace Columbus
{

	//////////////////////////////////////////////////////////////////////////////
	//Constructor
	C_Render::C_Render()
	{
	}
	//////////////////////////////////////////////////////////////////////////////
	//Enable all OpenGL varyables
	void C_Render::enableAll()
	{
		C_OpenStreamOpenGL(0);
		C_OpenStreamOpenGL(1);
		C_OpenStreamOpenGL(2);
		C_OpenStreamOpenGL(3);
		C_OpenStreamOpenGL(4);

		C_EnableDepthTestOpenGL();
		C_EnableBlendOpenGL();
		C_EnableAlphaTestOpenGL();
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Render::enableDepthPrepass()
	{
		glColorMask(0, 0, 0, 0);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Render::disableDepthPrepass()
	{
		glColorMask(1, 1, 1, 1);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Render::renderDepthPrepass(GameObject* aGameObject)
	{
		/*if (aGameObject == nullptr) return;
		if (aGameObject->hasComponent("MeshRenderer") == false &&
			aGameObject->hasComponent("ParticleSystem") == false) return;

		if (aGameObject->hasComponent("MeshRenderer"))
			if (gMeshWhiteShader == nullptr)
				gMeshWhiteShader = gDevice->createShader("Data/Shaders/standart.vert", "Data/Shaders/White.frag");

		if (aGameObject->hasComponent("ParticleSystem"))
			if (gParticleWhiteShader == nullptr)
				gParticleWhiteShader = gDevice->createShader("Data/Shaders/particle.vert", "Data/Shaders/White.frag");

		if (aGameObject->hasComponent("MeshRenderer"))
		{
			Shader* shaderPtr = static_cast<MeshRenderer*>(aGameObject->getComponent("MeshRenderer"))->getShader();
			static_cast<MeshRenderer*>(aGameObject->getComponent("MeshRenderer"))->setShader(gMeshWhiteShader);
			aGameObject->render();
			static_cast<MeshRenderer*>(aGameObject->getComponent("MeshRenderer"))->setShader(shaderPtr);
			return;
		}

		if (aGameObject->hasComponent("ParticleSystem"))
		{
			Shader* shaderPtr = static_cast<ParticleSystem*>(aGameObject->getComponent("ParticleSystem"))->getShader();
			static_cast<ParticleSystem*>(aGameObject->getComponent("ParticleSystem"))->setShader(gParticleWhiteShader);
			aGameObject->render();
			static_cast<ParticleSystem*>(aGameObject->getComponent("ParticleSystem"))->setShader(shaderPtr);
			return;
		}*/
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Render::render(GameObject* aGameObject)
	{
		/*if (aGameObject == nullptr) return;

		aGameObject->render();*/
	}
	//////////////////////////////////////////////////////////////////////////////
	//Destructor
	C_Render::~C_Render()
	{

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
						Meshes.push_back(std::make_pair(Mesh, Object.second->GetTransform()));
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

		auto ParticleSorter = [&](ParticleEmitter* A, ParticleEmitter* B)->bool
		{
			double ADistance, BDistance;

			APosition = A->getParticleEffect()->getPos();
			BPosition = B->getParticleEffect()->getPos();

			ADistance = pow(CameraPosition.X - APosition.X, 2) + pow(CameraPosition.Y - APosition.Y, 2) + pow(CameraPosition.Z - APosition.Z, 2);
			BDistance = pow(CameraPosition.X - BPosition.X, 2) + pow(CameraPosition.Y - BPosition.Y, 2) + pow(CameraPosition.Z - BPosition.Z, 2);

			return ADistance > BDistance;
		};

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
					ShaderProgram* CurrentShader;
					ShaderProgram* PreviousShader;

					for (auto& MeshRenderer : Meshes)
					{
						CurrentShader = MeshRenderer.first->GetMaterial().GetShader();

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

							MeshRenderer.first->Bind();
							PolygonsRendered += MeshRenderer.first->Render(MeshRenderer.second);
							MeshRenderer.first->Unbind();
						}

						PreviousShader = MeshRenderer.first->GetMaterial().GetShader();
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
