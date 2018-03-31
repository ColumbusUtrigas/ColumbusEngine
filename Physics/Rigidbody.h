#pragma once

#include <btBulletDynamicsCommon.h>
#include <Physics/PhysicsShape.h>
#include <Math/Vector3.h>
#include <Scene/Transform.h>
#include <Core/Types.h>

namespace Columbus
{

	class Rigidbody
	{
	private:
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
	public:
		btRigidBody* mRigidbody;
	public:
		Rigidbody(PhysicsShape* Shape = nullptr);
		Rigidbody(Transform Transform, PhysicsShape* Shape = nullptr);

		virtual void SetStatic(bool Static);
		virtual void SetTransform(Transform Transform);
		virtual void SetMass(float Mass);
		virtual void SetRestitution(float Restitution);
		virtual void SetFriction(float Friction);
		virtual void SetRollingFriction(float Friction);
		virtual void SetAngularDamping(float Damping);
		virtual void SetAngularTreshold(float Treshold);
		virtual void SetAngularFactor(Vector3 Factor);
		virtual void SetAngularVelocity(Vector3 Velocity);
		virtual void SetLinearDamping(float Damping);
		virtual void SetLinearTreshold(float Treshold);
		virtual void SetLinearFactor(Vector3 Factor);
		virtual void SetLinearVelocity(Vector3 Velocity);
		virtual void SetCollisionShape(PhysicsShape* Shape);

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
		virtual PhysicsShape* GetCollisionShape() const;

		~Rigidbody();
	};

}




