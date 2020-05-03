#pragma once

#include <btBulletDynamicsCommon.h>
#include <Physics/PhysicsShape.h>
#include <Math/Vector3.h>
#include <Scene/Transform.h>
#include <Core/Types.h>
#include <Core/ISerializable.h>

namespace Columbus
{

	class Rigidbody : public ISerializable
	{
	protected:
		bool Static;
		float Mass;
		float Restitution;
		float Friction;
		float RollingFriction;
		float AngularDamping;
		float AngularTreshold;
		Vector3 AngularFactor;
		float LinearTreshold;
		float LinearDamping;
		Vector3 LinearFactor;

		PhysicsShape* Shape;
		Transform Trans;
	public:
		btRigidBody* mRigidbody;
	public:
		Rigidbody(PhysicsShape* InShape = nullptr);
		Rigidbody(Transform InTransform, PhysicsShape* INShape = nullptr);

		virtual void Activate();
		virtual void ApplyCentralForce(Vector3 Force);
		virtual void ApplyCentralImpulse(Vector3 Impulse);
		virtual void ApplyForce(Vector3 Force, Vector3 RelPos);
		virtual void ApplyImpulse(Vector3 Impulse, Vector3 RelPos);
		virtual void ApplyTorque(Vector3 Torque);
		virtual void ApplyTorqueImpulse(Vector3 Torque);

		virtual void SetStatic(bool InStatic);
		virtual void SetTransform(Transform InTransform);
		virtual void SetMass(float InMass);
		virtual void SetRestitution(float InRestitution);
		virtual void SetFriction(float InFriction);
		virtual void SetRollingFriction(float InFriction);
		virtual void SetAngularDamping(float InDamping);
		virtual void SetAngularTreshold(float InTreshold);
		virtual void SetAngularFactor(Vector3 InFactor);
		virtual void SetAngularVelocity(Vector3 InVelocity);
		virtual void SetLinearDamping(float InDamping);
		virtual void SetLinearTreshold(float InTreshold);
		virtual void SetLinearFactor(Vector3 InFactor);
		virtual void SetLinearVelocity(Vector3 Velocity);
		virtual void SetGravity(Vector3 Gravity);
		virtual void SetCollisionShape(PhysicsShape* InShape);

		virtual bool IsStatic() const;
		virtual Transform GetTransform() const;
		virtual float GetMass() const;
		virtual float GetRestitution() const;
		virtual float GetFriction() const;
		virtual float GetRollingFriction() const;
		virtual float GetAngularDamping() const;
		virtual float GetAngularTreshold() const;
		virtual Vector3 GetAngularFactor() const;
		virtual Vector3 GetAngularVelocity() const;
		virtual float GetLinearDamping() const;
		virtual float GetLinearTreshold() const;
		virtual Vector3 GetLinearFactor() const;
		virtual Vector3 GetLinearVelocity() const;
		virtual Vector3 GetGravity() const;
		virtual PhysicsShape* GetCollisionShape() const;

		virtual void Serialize(JSON& J) const override;
		virtual void Deserialize(JSON& J) override;

		~Rigidbody();
	};

}




