#include <Physics/Rigidbody.h>

namespace Columbus
{

	Rigidbody::Rigidbody(PhysicsShape* Shape) :
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

		SetCollisionShape(Shape);
		SetAngularDamping(AngularDamping);
		SetLinearDamping(LinearDamping);
		SetAngularTreshold(AngularTreshold);
		SetLinearTreshold(LinearTreshold);
	}

	Rigidbody::Rigidbody(Transform Transform, PhysicsShape* Shape) :
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

		SetTransform(Transform);
		SetCollisionShape(Shape);
		SetAngularDamping(AngularDamping);
		SetLinearDamping(LinearDamping);
		SetAngularTreshold(AngularTreshold);
		SetLinearTreshold(LinearTreshold);
	}

	void Rigidbody::SetStatic(bool Static)
	{
		if (mRigidbody != nullptr)
		{
			this->Static = Static;

			if (Static == true)
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

	/*
	* Additional functions for angles convertation
	*/
	void MatrixToAngles(btMatrix3x3& InMatrix, Vector3& OutAngles)
	{
		auto Forward = InMatrix.getColumn(2);
		auto Left = InMatrix.getColumn(0);
		auto Up = InMatrix.getColumn(1);

		float Dist = sqrtf(Forward.getZ() * Forward.getZ() + Forward.getX() * Forward.getX());
		
		//Enough here to get angles?
		if (Dist > 0.001f)
		{
			// (yaw) y = ATAN(Forward.y, Forward.x);
			OutAngles.Y = Math::Degrees(atan2f(Forward.getX(), Forward.getZ()));

			// (pitch) x = ATAN(-Forward.z, sqrt(Forward.x * Forward.x + Forward.y * Forward.y));
			OutAngles.X = Math::Degrees(atan2f(-Forward.getY(), Dist));

			// (roll) z = ATAN(Left.z, Up.z);
			OutAngles.Z = Math::Degrees(atan2f(Left.getY(), Up.getY()));
		}
		else
		{
			// (yaw) y = ATAN(-Left.x, Left.y);
			OutAngles.Y = Math::Degrees(atan2f(-Left.getZ(), Left.getX()));

			// (pitch) x = ATAN(-Forward.z, sqrt(Forward.x * Forward.x + Forward.y * Forward.y));
			OutAngles.X = Math::Degrees(atan2f(-Forward.getY(), Dist));

			// Assume no roll in this case as one degree of freedom has been lost (i.e. yaw == roll)
			OutAngles.Z = 0;
		}
	}
	/*
	* End of additional functions for angles convertation
	*/

	void Rigidbody::SetTransform(Transform Transform)
	{
		if (mRigidbody != nullptr)
		{
			btTransform Trans;
			Vector3 pos = Transform.GetPos();
			Vector3 rot = Transform.GetRot();
			Vector3 scale = Transform.GetScale();

			if (rot.X > 180) rot.X -= 360;
			if (rot.Y > 180) rot.Y -= 360;
			if (rot.Z > 180) rot.Z -= 360;

			rot.X = Math::Radians(rot.Z);
			rot.Y = Math::Radians(rot.Y);
			rot.Z = Math::Radians(rot.Z);

			//btQuaternion quat; quat.setEulerZYX(rot.z, rot.y, rot.x);

			Trans.setOrigin(btVector3(pos.X, pos.Y, pos.Z));
			Trans.getBasis().setEulerZYX(rot.X, rot.Z, rot.Y);
			//Trans.setRotation(quat);
			mRigidbody->proceedToTransform(Trans);

			this->Trans = Transform;
		}
		//mShape->setLocalScaling(btVector3(scale.x, scale.y, scale.z)); //Hmmm
	}

	void Rigidbody::SetMass(float Mass)
	{
		if (mRigidbody != nullptr)
		{
			btVector3 Inertia;
			mRigidbody->getCollisionShape()->calculateLocalInertia(Mass, Inertia);
			mRigidbody->setMassProps(Mass, Inertia);
		}
	}

	void Rigidbody::SetRestitution(float Restitution)
	{
		if (mRigidbody != nullptr)
		{
			this->Restitution = Restitution;
			mRigidbody->setRestitution(Restitution);
		}
	}

	void Rigidbody::SetFriction(float Friction)
	{
		if (mRigidbody != nullptr)
		{
			this->Friction = Friction;
			mRigidbody->setFriction(Friction);
		}
	}

	void Rigidbody::SetRollingFriction(float Friction)
	{
		if (mRigidbody != nullptr)
		{
			this->RollingFriction = Friction;
			mRigidbody->setRollingFriction(Friction);
		}
	}

	void Rigidbody::SetAngularDamping(float Damping)
	{
		if (mRigidbody != nullptr)
		{
			AngularDamping = Damping;
			mRigidbody->setDamping(LinearDamping, AngularDamping);
		}
	}

	void Rigidbody::SetAngularTreshold(float Treshold)
	{
		if (mRigidbody != nullptr)
		{
			AngularTreshold = Treshold;
			mRigidbody->setSleepingThresholds(LinearTreshold, AngularTreshold);
		}
	}

	void Rigidbody::SetAngularFactor(Vector3 Factor)
	{
		if (mRigidbody != nullptr)
		{
			AngularFactor = Factor;
			mRigidbody->setAngularFactor(btVector3(Factor.X, Factor.Y, Factor.Z));
		}
	}

	void Rigidbody::SetAngularVelocity(Vector3 Velocity)
	{
		if (mRigidbody != nullptr)
		{
			mRigidbody->setAngularVelocity(btVector3(Math::Radians(Velocity.X), Math::Radians(Velocity.Y), Math::Radians(Velocity.Z)));
		}
	}

	void Rigidbody::SetLinearDamping(float Damping)
	{
		if (mRigidbody != nullptr)
		{
			LinearDamping = Damping;
			mRigidbody->setDamping(LinearDamping, AngularDamping);
		}
	}

	void Rigidbody::SetLinearTreshold(float Treshold)
	{
		if (mRigidbody != nullptr)
		{
			LinearTreshold = Treshold;
			mRigidbody->setSleepingThresholds(LinearTreshold, AngularTreshold);
		}
	}

	void Rigidbody::SetLinearFactor(Vector3 Factor)
	{
		if (mRigidbody != nullptr)
		{
			LinearFactor = Factor;
			mRigidbody->setLinearFactor(btVector3(Factor.X, Factor.Y, Factor.Z));
		}
	}

	void Rigidbody::SetLinearVelocity(Vector3 Velocity)
	{
		if (mRigidbody != nullptr)
		{
			mRigidbody->setLinearVelocity(btVector3(Velocity.X, Velocity.Y, Velocity.Z));
		}
	}

	void Rigidbody::SetCollisionShape(PhysicsShape* Shape)
	{
		if (mRigidbody != nullptr)
		{
			if (Shape != nullptr)
			{
				if (Shape->mShape != nullptr)
				{
					this->Shape = Shape;
					mRigidbody->setCollisionShape(Shape->mShape);
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
		Transform Trans;

		if (mRigidbody != nullptr)
		{
			btTransform bTrans;
			btVector3 pos;
			Vector3 rot;

			bTrans = mRigidbody->getWorldTransform();

			pos = bTrans.getOrigin();

			MatrixToAngles(bTrans.getBasis(), rot);

			/*bTrans.getBasis().getEulerYPR(rot.x, rot.z, rot.y);

			rot.x = Degrees(rot.x);
			rot.y = Degrees(rot.y);
			rot.z = Degrees(rot.z);*/

			Trans.SetPos(Vector3(pos.getX(), pos.getY(), pos.getZ()));
			Trans.SetRot(Vector3(rot.X, rot.Y, rot.Z));
			Trans.SetScale(this->Trans.GetScale());
			Trans.Update(); //Hmmm

			(Transform)this->Trans = Trans;
		}

		return Trans;
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








