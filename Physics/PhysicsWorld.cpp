#include <Physics/PhysicsWorld.h>
#include <Profiling/Profiling.h>

namespace Columbus
{

	PhysicsWorld::PhysicsWorld()
	{
		mBroadphase = new btDbvtBroadphase();
		mCollisionConfiguration = new btDefaultCollisionConfiguration();
		mDispatcher = new btCollisionDispatcher(mCollisionConfiguration);
		mSolver = new btSequentialImpulseConstraintSolver();
		mWorld = new btDiscreteDynamicsWorld(mDispatcher, mBroadphase, mSolver, mCollisionConfiguration);
	}

	void PhysicsWorld::SetGravity(Vector3 aGravity)
	{
		mWorld->setGravity(btVector3(aGravity.X, aGravity.Y, aGravity.Z));
	}

	void PhysicsWorld::AddRigidbody(Rigidbody* aRigidbody)
	{
		if (aRigidbody != nullptr)
		{
			mWorld->addRigidBody(aRigidbody->mRigidbody);
		}
	}

	void PhysicsWorld::RemoveRigidbody(Rigidbody* aRigidbody)
	{
		if (aRigidbody != nullptr)
		{
			mWorld->removeRigidBody(aRigidbody->mRigidbody);
		}
	}

	void PhysicsWorld::Step(float aDeltaTime, int aSubSteps)
	{
		PROFILE_CPU(ProfileModule::Physics);
		mWorld->stepSimulation(aDeltaTime, aSubSteps);
	}

	void PhysicsWorld::ClearForces()
	{
		mWorld->clearForces();
	}

	PhysicsWorld::~PhysicsWorld()
	{
		delete mWorld;
		delete mSolver;
		delete mCollisionConfiguration;
		delete mDispatcher;
		delete mBroadphase;
	}

}


