#pragma once

#include <Physics/Rigidbody.h>
#include <btBulletDynamicsCommon.h>

namespace Columbus
{

	class PhysicsWorld
	{
	private:
		btBroadphaseInterface* mBroadphase;
		btDefaultCollisionConfiguration* mCollisionConfiguration;
		btCollisionDispatcher* mDispatcher;
		btSequentialImpulseConstraintSolver* mSolver;
		btDiscreteDynamicsWorld* mWorld;
	public:
		PhysicsWorld();

		virtual void SetGravity(Vector3 aGravity);
		virtual void AddRigidbody(Rigidbody* aRigidbody);
		virtual void RemoveRigidbody(Rigidbody* aRigidbody);
		virtual void Step(float aDeltaTime, int aSubSteps);

		~PhysicsWorld();
	};

}




