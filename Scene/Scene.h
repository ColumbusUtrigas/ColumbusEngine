#pragma once

#include <Scene/GameObject.h>
#include <Scene/ComponentAudioSource.h>
#include <Scene/ComponentLight.h>
#include <Scene/ComponentMeshRenderer.h>
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
		std::map<uint32, SmartPointer<Texture>> mTextures;
		std::map<uint32, SmartPointer<ShaderProgram>> ShaderPrograms;

		std::map<uint32, SmartPointer<Mesh>> Meshes;
		std::map<uint32, SmartPointer<Sound>> Sounds;

		Renderer Render;

		Timer DeltaTime;
		PhysicsWorld PhysWorld;

		Skybox* mSkybox = nullptr;
		Camera* mCamera = nullptr;
		AudioListener* Listener = nullptr;

		iVector2 ContextSize = iVector2(640, 480);

		void audioWorkflow();
		void lightWorkflow();
		void meshWorkflow();
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

		void SetSkybox(Skybox* Sky) { mSkybox = Sky; }
		void SetCamera(Camera* Cam) { mCamera = Cam; }
		void SetAudioListener(AudioListener* InListener) { Listener = InListener; }
		void SetContextSize(const iVector2& NewContextSize) { ContextSize = NewContextSize; }

		GameObject* getGameObject(const unsigned int aID) const;
		GameObject* getGameObject(const std::string aName) const;

		void update();
		void render();

		~Scene();
	};

}
