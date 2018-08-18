#pragma once

#include <Scene/GameObject.h>
#include <Scene/ComponentLight.h>
#include <Scene/ComponentMeshRenderer.h>
#include <Scene/ComponentMeshInstancedRenderer.h>
#include <Scene/ComponentParticleSystem.h>
#include <Scene/ComponentRigidbody.h>
#include <Audio/AudioSystem.h>
#include <Graphics/Skybox.h>
#include <Graphics/Camera.h>
#include <Graphics/Render.h>
#include <Graphics/Primitives.h>
#include <Graphics/PostEffect.h>
#include <Physics/PhysicsShape.h>
#include <Physics/PhysicsShapeBox.h>
#include <Physics/PhysicsShapeCapsule.h>
#include <Physics/PhysicsShapeCone.h>
#include <Physics/PhysicsShapeConvexHull.h>
#include <Physics/PhysicsShapeCylinder.h>
#include <Physics/PhysicsShapeMultiSphere.h>
#include <Physics/PhysicsShapeSphere.h>
#include <Physics/PhysicsWorld.h>
#include <Core/Types.h>
#include <Core/SmartPointer.h>

namespace Columbus
{

	class Scene
	{
	private:
		std::map<uint32, SmartPointer<GameObject>> mObjects;
		std::vector<Light*> mLights;
		std::map<uint32, Texture*> mTextures;
		std::map<uint32, ShaderProgram*> ShaderPrograms;

		std::map<uint32, std::vector<Vertex>> Meshes;

		Renderer Render;

		Timer DeltaTime;
		PhysicsWorld PhysWorld;

		Skybox* mSkybox = nullptr;
		Camera* mCamera = nullptr;
		AudioListener* Listener = nullptr;

		PostEffect mNoneEffect;
		ShaderProgram* NoneShader = nullptr;

		Vector2 mContextSize = Vector2(640, 480);

		void lightWorkflow();
		void meshWorkflow();
		void meshInstancedWorkflow();
		void particlesWorkflow();
		void rigidbodyWorkflow();
		void rigidbodyPostWorkflow();
	public:
		AudioSystem Audio;
	public:
		Scene();

		bool load(std::string aFile);

		void Add(uint32 ID, GameObject&& InObject)
		{
			mObjects.insert(std::make_pair(ID, SmartPointer<GameObject>(new GameObject(std::move(InObject)))));
		}

		void setSkybox(const Skybox* aSkybox);
		void setCamera(const Camera* aCamera);
		void SetAudioListener(AudioListener* InListener) { Listener = InListener; }
		void setContextSize(const Vector2 aContextSize);

		GameObject* getGameObject(const unsigned int aID) const;
		GameObject* getGameObject(const std::string aName) const;

		void update();
		void render();

		~Scene();
	};

}
