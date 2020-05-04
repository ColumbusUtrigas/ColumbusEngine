#include <Physics/Rigidbody.h>
#include <Common/JSON/JSON.h>

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
		LinearTreshold(0.2f),
		LinearDamping(0.2f),
		LinearFactor(1, 1, 1),
		Shape(nullptr),
		mRigidbody(nullptr)
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
		LinearTreshold(0.2f),
		LinearDamping(0.2f),
		LinearFactor(1, 1, 1),
		Shape(nullptr),
		mRigidbody(nullptr)
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

	void Rigidbody::Activate()
	{
		if (mRigidbody != nullptr)
		{
			mRigidbody->activate();
		}
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
			Vector3 pos = InTransform.Position;
			//Vector3 scale = InTransform.Scale;

			Quaternion Q = InTransform.Rotation;
			btQuaternion bQuat(-Q.X, -Q.Y, -Q.Z, Q.W);

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
			
			if (!Static)
			{
				btVector3 Inertia;
				mRigidbody->getCollisionShape()->calculateLocalInertia(InMass, Inertia);
				mRigidbody->setMassProps(InMass, Inertia);
			}
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
			if (InShape != nullptr && InShape->mShape != nullptr)
			{
				Shape = InShape;
				mRigidbody->setCollisionShape(InShape->mShape);

				SetStatic(Static);
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

			Result.Position = Vector3(bTrans.getOrigin().getX(), bTrans.getOrigin().getY(), bTrans.getOrigin().getZ());
			Result.Rotation = Quaternion(-bTrans.getRotation().getX(), -bTrans.getRotation().getY(), -bTrans.getRotation().getZ(), bTrans.getRotation().getW());
			Result.Scale = Trans.Scale;
			Result.Update();

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

	void Rigidbody::Serialize(JSON& J) const
	{
		J["Static"] = Static;
		J["Mass"] = Mass;
		J["Restitution"] = Restitution;
		J["Friction"] = Friction;
		J["RollingFriction"] = RollingFriction;
		J["AngularDamping"] = AngularDamping;
		J["AngularTreshold"] = AngularTreshold;
		J["AngularFactor"] = AngularFactor;
		J["LinearTreshold"] = LinearTreshold;
		J["LinearDamping"] = LinearDamping;
		J["LinearFactor"] = LinearFactor;
		J["Shape"] = Shape;
	}

	void Rigidbody::Deserialize(JSON& J)
	{
		Static = J["Static"].GetBool();
		Mass = J["Mass"].GetFloat();
		Restitution = J["Restitution"].GetFloat();
		Friction = J["Friction"].GetFloat();
		RollingFriction = J["RollingFriction"].GetFloat();
		AngularDamping = J["AngularDamping"].GetFloat();
		AngularTreshold = J["AngularTreshold"].GetFloat();
		AngularFactor = J["AngularFactor"].GetVector3<float>();
		LinearTreshold = J["LinearTreshold"].GetFloat();
		LinearDamping = J["LinearDamping"].GetFloat();
		LinearFactor = J["LinearFactor"].GetVector3<float>();

		if (J.HasChild("Shape") && !J["Shape"].IsNull())
		{
			auto type = J["Shape"]["Type"].GetString();
			auto shape = PrototypeFactory<PhysicsShape>::Instance().CreateFromTypename(type);
			if (shape != nullptr)
			{
				shape->Deserialize(J["Shape"]);
				auto clone = shape->Clone();
				SetCollisionShape(static_cast<PhysicsShape*>(clone));
			}
			else
			{
				Shape = nullptr;
			}
		}
		else
		{
			//mRigidbody->setCollisionShape();
			Shape = nullptr;
		}
	}

	Rigidbody::~Rigidbody()
	{
		delete mRigidbody->getMotionState();
		delete mRigidbody;
	}

}
