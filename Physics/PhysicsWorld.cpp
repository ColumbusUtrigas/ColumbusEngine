#include <Physics/PhysicsWorld.h>
#include <Profiling/Profiling.h>

IMPLEMENT_CPU_PROFILING_COUNTER("Physics Update", "Physics", CPUCounter_PhysicsStep);

#define PHYSICS_MULTITHREADED 1

#if PHYSICS_MULTITHREADED
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorldMt.h>
#include <BulletCollision/CollisionDispatch/btCollisionDispatcherMt.h>
#endif

namespace Columbus
{

	PhysicsWorld::PhysicsWorld()
	{
#if PHYSICS_MULTITHREADED
		btITaskScheduler* TaskScheduler = btCreateDefaultTaskScheduler();
		btSetTaskScheduler(TaskScheduler);
#endif

		mBroadphase = new btDbvtBroadphase();
		mCollisionConfiguration = new btDefaultCollisionConfiguration();

#if PHYSICS_MULTITHREADED
		mDispatcher = new btCollisionDispatcherMt(mCollisionConfiguration);
		mSolver = new btConstraintSolverPoolMt(8);
		mWorld = new btDiscreteDynamicsWorldMt(mDispatcher, mBroadphase, (btConstraintSolverPoolMt*)mSolver, NULL, mCollisionConfiguration);
#else
		mDispatcher = new btCollisionDispatcher(mCollisionConfiguration);
		mSolver = new btSequentialImpulseConstraintSolver();
		mWorld = new btDiscreteDynamicsWorld(mDispatcher, mBroadphase, mSolver, mCollisionConfiguration);
#endif
	}

	void PhysicsWorld::SetGravity(Vector3 aGravity)
	{
		mWorld->setGravity(btVector3(aGravity.X, aGravity.Y, aGravity.Z));
	}

	void PhysicsWorld::AddRigidbody(Rigidbody* aRigidbody)
	{
		mWorld->addRigidBody(aRigidbody->mRigidbody);
	}

	void PhysicsWorld::RemoveRigidbody(Rigidbody* aRigidbody)
	{
		mWorld->removeRigidBody(aRigidbody->mRigidbody);
	}

	void PhysicsWorld::AddRigidbody(btRigidBody* aRigidbody)
	{
		mWorld->addRigidBody(aRigidbody);
	}

	void PhysicsWorld::RemoveRigidbody(btRigidBody* aRigidbody)
	{
		mWorld->removeRigidBody(aRigidbody);
	}

	void PhysicsWorld::Step(float aDeltaTime, int aSubSteps)
	{
		PROFILE_CPU(CPUCounter_PhysicsStep);

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


