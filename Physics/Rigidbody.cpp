#include <Physics/Rigidbody.h>

namespace Columbus
{

	Rigidbody::Rigidbody(PhysicsShape* InShape) :
		Static(false),
		Mass(1.0f),
		Restitution(0.0f),
		Friction(0.5f),
		RollingFriction(0.0f),
		AngularDamping(0.2f),
		AngularTreshold(0.25f),
		AngularFactor(1, 1, 1),
		LinearDamping(0.2f),
		LinearTreshold(0.2),
		LinearFactor(1, 1, 1)
	{
		btDefaultMotionState* MotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));
		btRigidBody::btRigidBodyConstructionInfo* CI = new btRigidBody::btRigidBodyConstructionInfo(1, MotionState, new btSphereShape(1), btVector3(0, 0, 0));
		mRigidbody = new btRigidBody(*CI);

		SetCollisionShape(InShape);
		SetAngularDamping(AngularDamping);
		SetLinearDamping(LinearDamping);
		SetAngularTreshold(AngularTreshold);
		SetLinearTreshold(LinearTreshold);
	}

	Rigidbody::Rigidbody(Transform InTransform, PhysicsShape* InShape) :
		Static(false),
		Mass(1.0f),
		Restitution(0.0f),
		Friction(0.5f),
		RollingFriction(0.0f),
		AngularDamping(0.2f),
		AngularTreshold(0.25f),
		AngularFactor(1, 1, 1),
		LinearDamping(0.2f),
		LinearTreshold(0.2),
		LinearFactor(1, 1, 1)
	{
		btDefaultMotionState* MotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));
		btRigidBody::btRigidBodyConstructionInfo* CI = new btRigidBody::btRigidBodyConstructionInfo(1, MotionState, new btSphereShape(1), btVector3(0, 0, 0));
		mRigidbody = new btRigidBody(*CI);

		SetTransform(InTransform);
		SetCollisionShape(InShape);
		SetAngularDamping(AngularDamping);
		SetLinearDamping(LinearDamping);
		SetAngularTreshold(AngularTreshold);
		SetLinearTreshold(LinearTreshold);
	}

	void Rigidbody::ApplyCentralForce(Vector3 Force)
	{
		if (mRigidbody != nullptr)
		{
			mRigidbody->activate();
			mRigidbody->applyCentralForce(btVector3(Force.X, Force.Y, Force.Z));
		}
	}

	void Rigidbody::ApplyCentralImpulse(Vector3 Impulse)
	{
		if (mRigidbody != nullptr)
		{
			mRigidbody->activate();
			mRigidbody->applyCentralImpulse(btVector3(Impulse.X, Impulse.Y, Impulse.Z));
		}
	}

	void Rigidbody::ApplyForce(Vector3 Force, Vector3 RelPos)
	{
		if (mRigidbody != nullptr)
		{
			mRigidbody->activate();
			mRigidbody->applyForce(btVector3(Force.X, Force.Y, Force.Z), btVector3(RelPos.X, RelPos.Y, RelPos.Z));
		}
	}

	void Rigidbody::ApplyImpulse(Vector3 Impulse, Vector3 RelPos)
	{
		if (mRigidbody != nullptr)
		{
			mRigidbody->activate();
			mRigidbody->applyImpulse(btVector3(Impulse.X, Impulse.Y, Impulse.Z), btVector3(RelPos.X, RelPos.Y, RelPos.Z));
		}
	}

	void Rigidbody::ApplyTorque(Vector3 Torque)
	{
		if (mRigidbody != nullptr)
		{
			mRigidbody->activate();
			mRigidbody->applyTorque(btVector3(Torque.X, Torque.Y, Torque.Z));
		}
	}

	void Rigidbody::ApplyTorqueImpulse(Vector3 Torque)
	{
		if (mRigidbody != nullptr)
		{
			mRigidbody->activate();
			mRigidbody->applyTorqueImpulse(btVector3(Torque.X, Torque.Y, Torque.Z));
		}
	}

	void Rigidbody::SetStatic(bool InStatic)
	{
		if (mRigidbody != nullptr)
		{
			Static = InStatic;

			if (InStatic == true)
			{
				mRigidbody->setMassProps(0, btVector3(0, 0, 0));
			} else
			{
				SetMass(Mass);
				SetAngularVelocity(Vector3(0, 0, 0));
				SetLinearVelocity(Vector3(0, 0, 0));
			}
		}
	}

	void Rigidbody::SetTransform(Transform InTransform)
	{
		if (mRigidbody != nullptr)
		{
			btTransform bTrans;
			Vector3 pos = InTransform.GetPos();
			//Vector3 scale = InTransform.GetScale();

			glm::quat tQuat = InTransform.RotationQuaternion;
			btQuaternion bQuat(-tQuat.z, -tQuat.w, tQuat.x, -tQuat.y);

			bTrans.setOrigin(btVector3(pos.X, pos.Y, pos.Z));
			bTrans.setRotation(bQuat);
			mRigidbody->proceedToTransform(bTrans);

			Trans = InTransform;
		}
	}

	void Rigidbody::SetMass(float InMass)
	{
		if (mRigidbody != nullptr)
		{
			Mass = InMass;
			btVector3 Inertia;
			mRigidbody->getCollisionShape()->calculateLocalInertia(InMass, Inertia);
			mRigidbody->setMassProps(InMass, Inertia);
		}
	}

	void Rigidbody::SetRestitution(float InRestitution)
	{
		if (mRigidbody != nullptr)
		{
			Restitution = InRestitution;
			mRigidbody->setRestitution(InRestitution);
		}
	}

	void Rigidbody::SetFriction(float InFriction)
	{
		if (mRigidbody != nullptr)
		{
			Friction = InFriction;
			mRigidbody->setFriction(InFriction);
		}
	}

	void Rigidbody::SetRollingFriction(float InFriction)
	{
		if (mRigidbody != nullptr)
		{
			RollingFriction = InFriction;
			mRigidbody->setRollingFriction(InFriction);
		}
	}

	void Rigidbody::SetAngularDamping(float InDamping)
	{
		if (mRigidbody != nullptr)
		{
			AngularDamping = InDamping;
			mRigidbody->setDamping(LinearDamping, AngularDamping);
		}
	}

	void Rigidbody::SetAngularTreshold(float InTreshold)
	{
		if (mRigidbody != nullptr)
		{
			AngularTreshold = InTreshold;
			mRigidbody->setSleepingThresholds(LinearTreshold, AngularTreshold);
		}
	}

	void Rigidbody::SetAngularFactor(Vector3 InFactor)
	{
		if (mRigidbody != nullptr)
		{
			AngularFactor = InFactor;
			mRigidbody->setAngularFactor(btVector3(InFactor.X, InFactor.Y, InFactor.Z));
		}
	}

	void Rigidbody::SetAngularVelocity(Vector3 Velocity)
	{
		if (mRigidbody != nullptr)
		{
			mRigidbody->activate();
			mRigidbody->setAngularVelocity(btVector3(Math::Radians(Velocity.X), Math::Radians(Velocity.Y), Math::Radians(Velocity.Z)));
		}
	}

	void Rigidbody::SetLinearDamping(float InDamping)
	{
		if (mRigidbody != nullptr)
		{
			LinearDamping = InDamping;
			mRigidbody->setDamping(LinearDamping, AngularDamping);
		}
	}

	void Rigidbody::SetLinearTreshold(float InTreshold)
	{
		if (mRigidbody != nullptr)
		{
			LinearTreshold = InTreshold;
			mRigidbody->setSleepingThresholds(LinearTreshold, AngularTreshold);
		}
	}

	void Rigidbody::SetLinearFactor(Vector3 InFactor)
	{
		if (mRigidbody != nullptr)
		{
			LinearFactor = InFactor;
			mRigidbody->setLinearFactor(btVector3(InFactor.X, InFactor.Y, InFactor.Z));
		}
	}

	void Rigidbody::SetLinearVelocity(Vector3 Velocity)
	{
		if (mRigidbody != nullptr)
		{
			mRigidbody->activate();
			mRigidbody->setLinearVelocity(btVector3(Velocity.X, Velocity.Y, Velocity.Z));
		}
	}

	void Rigidbody::SetGravity(Vector3 Gravity)
	{
		if (mRigidbody != nullptr)
		{
			mRigidbody->setGravity(btVector3(Gravity.X, Gravity.Y, Gravity.Z));
			mRigidbody->applyGravity();
		}
	}

	void Rigidbody::SetCollisionShape(PhysicsShape* InShape)
	{
		if (mRigidbody != nullptr)
		{
			if (InShape != nullptr)
			{
				if (InShape->mShape != nullptr)
				{
					Shape = InShape;
					mRigidbody->setCollisionShape(InShape->mShape);
				}
			}
		}
	}

	bool Rigidbody::IsStatic() const
	{
		return this->Static;
	}

	Transform Rigidbody::GetTransform() const
	{
		Transform Result;

		if (mRigidbody != nullptr)
		{
			btTransform bTrans;

			bTrans = mRigidbody->getWorldTransform();
			//mRigidbody->getMotionState()->getWorldTransform(bTrans);

			Result.SetPos(Vector3(bTrans.getOrigin().getX(), bTrans.getOrigin().getY(), bTrans.getOrigin().getZ()));
			Result.SetRot(glm::quat(-bTrans.getRotation().getZ(), -bTrans.getRotation().getY(), -bTrans.getRotation().getX(), bTrans.getRotation().getW()));
			Result.SetScale(Trans.GetScale());
			Result.Update(); //Hmmm

			glm::vec3 rot(-3.141592653, 0, 0);
			Result.RotationQuaternion = glm::normalize(Result.RotationQuaternion * glm::quat(rot));

			(Transform)this->Trans = Result;
		}

		return Result;
	}

	float Rigidbody::GetMass() const
	{
		return Mass;
	}

	float Rigidbody::GetRestitution() const
	{
		return Restitution;
	}

	float Rigidbody::GetFriction() const
	{
		return Friction;
	}

	float Rigidbody::GetRollingFriction() const
	{
		return RollingFriction;
	}

	float Rigidbody::GetAngularDamping() const
	{
		return AngularDamping;
	}

	float Rigidbody::GetAngularTreshold() const
	{
		return AngularTreshold;
	}

	Vector3 Rigidbody::GetAngularFactor() const
	{
		return AngularFactor;
	}

	Vector3 Rigidbody::GetAngularVelocity() const
	{
		if (mRigidbody != nullptr)
		{
			btVector3 Velocity = mRigidbody->getAngularVelocity();
			return Vector3(Velocity.getX(), Velocity.getY(), Velocity.getZ());
		}

		return Vector3(0, 0, 0);
	}

	float Rigidbody::GetLinearDamping() const
	{
		return LinearDamping;
	}

	float Rigidbody::GetLinearTreshold() const
	{
		return LinearTreshold;
	}

	Vector3 Rigidbody::GetLinearFactor() const
	{
		return LinearFactor;
	}

	Vector3 Rigidbody::GetLinearVelocity() const
	{
		if (mRigidbody != nullptr)
		{
			btVector3 Velocity = mRigidbody->getLinearVelocity();
			return Vector3(Velocity.getX(), Velocity.getY(), Velocity.getZ());
		}

		return Vector3(0, 0, 0);
	}

	Vector3 Rigidbody::GetGravity() const
	{
		if (mRigidbody != nullptr)
		{
			btVector3 Gravity = mRigidbody->getGravity();
			return Vector3(Gravity.getX(), Gravity.getY(), Gravity.getZ());
		}

		return Vector3(0, 0, 0);
	}

	PhysicsShape* Rigidbody::GetCollisionShape() const
	{
		return this->Shape;
	}

	Rigidbody::~Rigidbody()
	{
		delete mRigidbody->getMotionState();
		delete mRigidbody;
	}

}








