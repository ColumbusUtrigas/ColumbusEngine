#pragma once

#include <Physics/Rigidbody.h>
#include <Profiling/Profiling.h>
#include <btBulletDynamicsCommon.h>

DECLARE_CPU_PROFILING_COUNTER(CPUCounter_PhysicsStep);

namespace Columbus
{

	class PhysicsWorld
	{
	public:
		btBroadphaseInterface* mBroadphase;
		btDefaultCollisionConfiguration* mCollisionConfiguration;
		btDispatcher* mDispatcher;
		btConstraintSolver* mSolver;
		btDiscreteDynamicsWorld* mWorld;
	public:
		PhysicsWorld();

		void SetGravity(Vector3 aGravity);

		void AddRigidbody(Rigidbody* aRigidbody);
		void RemoveRigidbody(Rigidbody* aRigidbody);

		void AddRigidbody(btRigidBody* aRigidbody);
		void RemoveRigidbody(btRigidBody* aRigidbody);

		void Step(float aDeltaTime, int aSubSteps);
		void ClearForces();

		~PhysicsWorld();
	};

}




