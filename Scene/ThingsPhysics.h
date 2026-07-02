#pragma once

#include "Graphics/World.h"

#include <BulletDynamics/ConstraintSolver/btConeTwistConstraint.h>
#include <BulletDynamics/ConstraintSolver/btFixedConstraint.h>
#include <BulletDynamics/ConstraintSolver/btHingeConstraint.h>
#include <BulletDynamics/ConstraintSolver/btPoint2PointConstraint.h>
#include <BulletDynamics/ConstraintSolver/btSliderConstraint.h>

namespace Columbus
{

///////////////////////////////////////////////////////////////////////////////////////////////////
// Invisible Walls

struct AInvisibleWall : public AThing
{
	CREFLECT_BODY_STRUCT_VIRTUAL(AInvisibleWall);
	using Super = AThing;

public:
	bool bEnabled = true;
	HCollisionShapeDesc CollisionShape;
	float Restitution = 0.0f;
	float Friction = 0.5f;
	float RollingFriction = 0.0f;

protected:
	Rigidbody* PhysicsBody = nullptr;
	btCollisionShape* Shape = nullptr;

public:
	AInvisibleWall();

	virtual void OnCreate() override;
	virtual void OnDestroy() override;
	virtual void OnUpdateRenderState() override;
	virtual void OnUiPropertyChange() override;

protected:
	void CreatePhysicsState();
	void DestroyPhysicsState();
	void UpdatePhysicsTransform();
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Constraints

enum class EConstraintAxis
{
	X,
	Y,
	Z,
};

struct AConstraint : public AThing
{
	CREFLECT_BODY_STRUCT_VIRTUAL(AConstraint);
	using Super = AThing;

public:
	ThingRef<AMeshInstance> BodyA;
	ThingRef<AMeshInstance> BodyB;

	bool bDisableCollisionBetweenBodies = true;
	bool bCanBreak = false;
	float BreakingImpulseThreshold = 1000.0f;
	bool bKeepBodiesAwake = false;

protected:
	btTypedConstraint* Constraint = nullptr;

public:
	AConstraint();

	virtual void OnCreate() override;
	virtual void OnDestroy() override;
	virtual void OnTick(float DeltaTime) override;
	virtual void OnUiPropertyChange() override;

	bool UpdateCachedFrames(EConstraintAxis AuthorAxis, EConstraintAxis BulletAxisColumn);
	bool HasCachedFrames() const;
	Matrix GetBodyAFrameWorld() const;
	Matrix GetBodyBFrameWorld() const;

protected:
	Matrix BodyAFrameLocal = Matrix(1.0f);
	Matrix BodyBFrameLocal = Matrix(1.0f);
	bool bHasCachedFrames = false;

	virtual btTypedConstraint* CreateConstraint();
	virtual bool NeedsRuntimeTick() const;

	bool TryCreateConstraint();
	void DestroyConstraint();
	void WakeBodies();

	btRigidBody* GetBodyARigidbody();
	btRigidBody* GetBodyBRigidbody();
	Matrix BuildBodyLocalFrameMatrix(AMeshInstance* Body, const Matrix& ConstraintFrameWorld);
	Matrix BuildConstraintFrameWorld(EConstraintAxis AuthorAxis, EConstraintAxis BulletAxisColumn);
};

struct AFixedConstraint : public AConstraint
{
	CREFLECT_BODY_STRUCT_VIRTUAL(AFixedConstraint);
	using Super = AConstraint;

protected:
	virtual btTypedConstraint* CreateConstraint() override;
};

struct APointConstraint : public AConstraint
{
	CREFLECT_BODY_STRUCT_VIRTUAL(APointConstraint);
	using Super = AConstraint;

protected:
	virtual btTypedConstraint* CreateConstraint() override;
};

struct ASliderConstraint : public AConstraint
{
	CREFLECT_BODY_STRUCT_VIRTUAL(ASliderConstraint);
	using Super = AConstraint;

public:
	EConstraintAxis Axis = EConstraintAxis::X;

	float LowerLinearLimit = -1.0f;
	float UpperLinearLimit = 1.0f;
	float LowerAngularLimitDegrees = 0.0f;
	float UpperAngularLimitDegrees = 0.0f;

	bool  bEnableLinearMotor = false;
	float LinearMotorTargetVelocity = 0.0f;
	float MaxLinearMotorForce = 20.0f;

	bool  bEnableAngularMotor = false;
	float AngularMotorTargetVelocityDegrees = 0.0f;
	float MaxAngularMotorForce = 20.0f;

	Matrix GetFrameWorld();
	Vector3 GetAxisWorld();

protected:
	virtual btTypedConstraint* CreateConstraint() override;
	virtual bool NeedsRuntimeTick() const override;
};

struct AHingeConstraint : public AConstraint
{
	CREFLECT_BODY_STRUCT_VIRTUAL(AHingeConstraint);
	using Super = AConstraint;

public:
	EConstraintAxis Axis = EConstraintAxis::Y;

	bool  bUseLimits = true;
	float LowerLimitDegrees     = -90.0f;
	float UpperLimitDegrees     = 90.0f;
	float LimitSoftness         = 0.9f;
	float LimitBiasFactor       = 0.3f;
	float LimitRelaxationFactor = 1.0f;

	bool  bEnableMotor = false;
	float MotorTargetVelocityDegrees = 0.0f;
	float MaxMotorImpulse = 20.0f;

	Matrix GetFrameWorld();
	Matrix GetBodyAFrameWorldForCurrentPose();

protected:
	virtual btTypedConstraint* CreateConstraint() override;
	virtual bool NeedsRuntimeTick() const override;
};

struct AConeTwistConstraint : public AConstraint
{
	CREFLECT_BODY_STRUCT_VIRTUAL(AConeTwistConstraint);
	using Super = AConstraint;

public:
	EConstraintAxis TwistAxis   = EConstraintAxis::X;

	float SwingSpan1Degrees     = 45.0f;
	float SwingSpan2Degrees     = 45.0f;
	float TwistSpanDegrees      = 45.0f;
	float LimitSoftness         = 0.9f;
	float LimitBiasFactor       = 0.3f;
	float LimitRelaxationFactor = 1.0f;

protected:
	virtual btTypedConstraint* CreateConstraint() override;
};

}

CREFLECT_DECLARE_STRUCT_WITH_PARENT_VIRTUAL(Columbus::AInvisibleWall, Columbus::AThing, 1, "2D9C9E5D-8A89-4627-8D6D-75594147E4F9");

CREFLECT_DECLARE_ENUM(Columbus::EConstraintAxis, "075EF12B-F95D-4D4B-BCBD-196E6AD9F742");
CREFLECT_DECLARE_STRUCT_WITH_PARENT_VIRTUAL(Columbus::AConstraint, Columbus::AThing, 1, "6F13F262-CA67-43DA-A9E2-BD8C33B34838");
CREFLECT_DECLARE_STRUCT_WITH_PARENT_VIRTUAL(Columbus::AFixedConstraint, Columbus::AConstraint, 1, "8F7DB696-05ED-49D3-9E70-8B7C3A3DA6C3");
CREFLECT_DECLARE_STRUCT_WITH_PARENT_VIRTUAL(Columbus::APointConstraint, Columbus::AConstraint, 1, "55D25DB4-C123-40D4-BAF5-9E4F87454771");
CREFLECT_DECLARE_STRUCT_WITH_PARENT_VIRTUAL(Columbus::ASliderConstraint, Columbus::AConstraint, 1, "38F65F10-A245-43E2-B381-6471116C3F66");
CREFLECT_DECLARE_STRUCT_WITH_PARENT_VIRTUAL(Columbus::AHingeConstraint, Columbus::AConstraint, 4, "E09B638B-94DF-4F7F-B8DC-290EDB19175E");
CREFLECT_DECLARE_STRUCT_WITH_PARENT_VIRTUAL(Columbus::AConeTwistConstraint, Columbus::AConstraint, 1, "05291C9D-E493-43A9-98AE-120EBCF11EA3");
