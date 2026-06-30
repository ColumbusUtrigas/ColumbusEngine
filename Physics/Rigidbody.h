#pragma once

#include <btBulletDynamicsCommon.h>
#include <Physics/PhysicsShape.h>
#include <Math/Vector3.h>
#include <Scene/Transform.h>
#include <Core/Types.h>

namespace Columbus
{

	struct Mesh2;

	class Rigidbody
	{
	protected:
		ECollisionMotionType MotionType = ECollisionMotionType::Static;
		float Mass = 1.0f;
		float Restitution = 0.0f;
		float Friction = 0.5f;
		float RollingFriction = 0.0f;
		float AngularDamping = 0.2f;
		float AngularTreshold = 0.25f;
		Vector3 AngularFactor = Vector3(1, 1, 1);
		float LinearTreshold = 0.2f;
		float LinearDamping = 0.2f;
		Vector3 LinearFactor = Vector3(1, 1, 1);

		Transform Trans;
	public:
		btRigidBody* mRigidbody;
	public:
		Rigidbody(btCollisionShape* InShape);

		void SetCollisionSettings(const HCollisionSettings& Settings);

		virtual void Activate();
		virtual void ApplyCentralForce(Vector3 Force);
		virtual void ApplyCentralImpulse(Vector3 Impulse);
		virtual void ApplyForce(Vector3 Force, Vector3 RelPos);
		virtual void ApplyImpulse(Vector3 Impulse, Vector3 RelPos);
		virtual void ApplyTorque(Vector3 Torque);
		virtual void ApplyTorqueImpulse(Vector3 Torque);

		virtual void SetMotionType(ECollisionMotionType InMotionType);
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

		virtual bool IsStatic() const;
		virtual bool IsKinematic() const;
		virtual bool IsDynamic() const;
		virtual ECollisionMotionType GetMotionType() const;
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

		~Rigidbody();
	};

}




