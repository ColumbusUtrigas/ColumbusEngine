#include "Scene/ThingsPhysics.h"

#include <System/Log.h>

namespace Columbus
{

///////////////////////////////////////////////////////////////////////////////////////////////////
// Helpers

static btTransform ToBulletTransform(const Transform& InTransform)
{
	btTransform Result;
	Result.setIdentity();

	const Vector3 Pos = InTransform.Position;
	const Quaternion Rot = InTransform.Rotation;

	Result.setOrigin(btVector3(Pos.X, Pos.Y, Pos.Z));
	Result.setRotation(btQuaternion(-Rot.X, -Rot.Y, -Rot.Z, Rot.W));
	return Result;
}

static btTransform ToBulletTransform(const Matrix& InMatrix)
{
	Transform InTransform;
	InTransform.SetFromMatrix(InMatrix);
	return ToBulletTransform(InTransform);
}

static Matrix ToEngineMatrix(const btTransform& InTransform)
{
	Transform Result;
	const btVector3 Pos = InTransform.getOrigin();
	const btQuaternion Rot = InTransform.getRotation();

	Result.Position = Vector3(Pos.getX(), Pos.getY(), Pos.getZ());
	Result.Rotation = Quaternion(-Rot.getX(), -Rot.getY(), -Rot.getZ(), Rot.getW());
	Result.Scale = Vector3(1.0f);
	Result.Update();
	return Result.GetMatrix();
}

static Vector3 GetFrameAxis(const Matrix& Frame, EConstraintAxis Axis)
{
	switch (Axis)
	{
	case EConstraintAxis::X: return Frame.GetColumn(0).XYZ().Normalized();
	case EConstraintAxis::Y: return Frame.GetColumn(1).XYZ().Normalized();
	case EConstraintAxis::Z: return Frame.GetColumn(2).XYZ().Normalized();
	}

	return Vector3(0, 1, 0);
}

static void SetFrameAxis(Matrix& Frame, EConstraintAxis AxisColumn, const Vector3& Axis)
{
	switch (AxisColumn)
	{
	case EConstraintAxis::X:
		Frame.SetColumn(0, Vector4(Axis, 0));
		break;
	case EConstraintAxis::Y:
		Frame.SetColumn(1, Vector4(Axis, 0));
		break;
	case EConstraintAxis::Z:
		Frame.SetColumn(2, Vector4(Axis, 0));
		break;
	}
}

static Vector3 SelectBasisReferenceAxis(const Vector3& Axis)
{
	return Math::Abs(Vector3::Dot(Axis, Vector3(0, 1, 0))) < 0.999f ? Vector3(0, 1, 0) : Vector3(1, 0, 0);
}

static Matrix BuildOrthonormalBasisFromAxis(const Vector3& Axis, EConstraintAxis AxisColumn)
{
	const Vector3 BasisZ = Axis.Normalized();
	const Vector3 ReferenceAxis = SelectBasisReferenceAxis(BasisZ);
	const Vector3 BasisX = Vector3::Cross(ReferenceAxis, BasisZ).Normalized();
	const Vector3 BasisY = Vector3::Cross(BasisZ, BasisX).Normalized();

	Matrix Frame(1.0f);
	SetFrameAxis(Frame, AxisColumn, BasisZ);

	switch (AxisColumn)
	{
	case EConstraintAxis::X:
		Frame.SetColumn(1, Vector4(BasisX, 0));
		Frame.SetColumn(2, Vector4(BasisY, 0));
		return Frame;
	case EConstraintAxis::Y:
		Frame.SetColumn(0, Vector4(BasisY, 0));
		Frame.SetColumn(2, Vector4(BasisX, 0));
		return Frame;
	case EConstraintAxis::Z:
		Frame.SetColumn(0, Vector4(BasisX, 0));
		Frame.SetColumn(1, Vector4(BasisY, 0));
		return Frame;
	}

	return Frame;
}

static Matrix BuildConstraintFrameWorldFromAxis(const Transform& WorldTransform, EConstraintAxis AuthorAxis, EConstraintAxis BulletAxisColumn)
{
	// AuthorAxis is selected in the constraint thing's transform. BulletAxisColumn is the local
	// frame column that the corresponding Bullet constraint treats as its operative axis.
	const Matrix TransformMatrix = WorldTransform.GetMatrix();
	const Vector3 Axis = GetFrameAxis(TransformMatrix, AuthorAxis);
	Matrix Frame = BuildOrthonormalBasisFromAxis(Axis, BulletAxisColumn);
	Frame.SetColumn(3, Vector4(WorldTransform.Position, 1));
	return Frame;
}

static bool IsInvisibleWallShapeSupported(const HCollisionShapeDesc& Desc)
{
	switch (Desc.Type)
	{
	case ECollisionShape::None:
	case ECollisionShape::Box:
	case ECollisionShape::Sphere:
	case ECollisionShape::Capsule:
	case ECollisionShape::Cone:
	case ECollisionShape::Cylinder:
		return true;
	case ECollisionShape::Compound:
		for (const HCollisionShapeDesc& Child : Desc.ChildShapes)
		{
			if (!IsInvisibleWallShapeSupported(Child))
			{
				return false;
			}
		}
		return true;
	case ECollisionShape::ConvexHull:
	case ECollisionShape::TriMesh:
		return false;
	}

	return false;
}

static void DeleteCollisionShapeTree(btCollisionShape* Shape)
{
	if (Shape == nullptr)
	{
		return;
	}

	if (Shape->isCompound())
	{
		btCompoundShape* Compound = static_cast<btCompoundShape*>(Shape);
		for (int ChildIndex = 0; ChildIndex < Compound->getNumChildShapes(); ChildIndex++)
		{
			DeleteCollisionShapeTree(Compound->getChildShape(ChildIndex));
		}
	}

	delete Shape;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Invisible Walls

AInvisibleWall::AInvisibleWall()
{
	CollisionShape.Type = ECollisionShape::Box;
	CollisionShape.Size = Vector3(1.0f, 1.0f, 1.0f);
	bNeedsTicking = false;
}

void AInvisibleWall::OnCreate()
{
	Super::OnCreate();
	CreatePhysicsState();
}

void AInvisibleWall::OnDestroy()
{
	DestroyPhysicsState();
	Super::OnDestroy();
}

void AInvisibleWall::OnUpdateRenderState()
{
	Super::OnUpdateRenderState();
	UpdatePhysicsTransform();
}

void AInvisibleWall::OnUiPropertyChange()
{
	Super::OnUiPropertyChange();
	CreatePhysicsState();
}

void AInvisibleWall::CreatePhysicsState()
{
	DestroyPhysicsState();

	if (!bEnabled || World == nullptr || World->Physics.mWorld == nullptr || CollisionShape.Type == ECollisionShape::None)
	{
		return;
	}

	if (!IsInvisibleWallShapeSupported(CollisionShape))
	{
		Log::Warning("[InvisibleWall] %s uses a mesh-backed shape; use primitive or compound primitive shapes instead", Name.c_str());
		return;
	}

	Shape = Physics::CreatePhysicsShapeFromDesc(CollisionShape, nullptr);
	if (Shape == nullptr)
	{
		Log::Warning("[InvisibleWall] Failed to create collision shape for %s", Name.c_str());
		return;
	}

	PhysicsBody = new Rigidbody(Shape);
	PhysicsBody->mRigidbody->setUserPointer(this);
	PhysicsBody->SetMotionType(ECollisionMotionType::Static);
	PhysicsBody->SetRestitution(Restitution);
	PhysicsBody->SetFriction(Friction);
	PhysicsBody->SetRollingFriction(RollingFriction);
	PhysicsBody->SetTransform(GetWorldTransform());

	World->Physics.AddRigidbody(PhysicsBody);
	World->Physics.mWorld->updateSingleAabb(PhysicsBody->mRigidbody);
}

void AInvisibleWall::DestroyPhysicsState()
{
	if (PhysicsBody != nullptr)
	{
		if (World != nullptr && World->Physics.mWorld != nullptr)
		{
			World->Physics.RemoveRigidbody(PhysicsBody);
		}

		delete PhysicsBody;
		PhysicsBody = nullptr;
	}

	DeleteCollisionShapeTree(Shape);
	Shape = nullptr;
}

void AInvisibleWall::UpdatePhysicsTransform()
{
	if (PhysicsBody == nullptr)
	{
		return;
	}

	PhysicsBody->SetTransform(GetWorldTransform());

	if (World != nullptr && World->Physics.mWorld != nullptr)
	{
		World->Physics.mWorld->updateSingleAabb(PhysicsBody->mRigidbody);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Constraints

///////////////////////////////////////////////////////////////////////////////////////////////////
// AConstraint

AConstraint::AConstraint()
{
	bNeedsTicking = true;
}

void AConstraint::OnCreate()
{
	Super::OnCreate();

	if (World == nullptr || World->WorldType != EWorldType::Game)
	{
		bNeedsTicking = false;
		return;
	}

	TryCreateConstraint();
}

void AConstraint::OnDestroy()
{
	DestroyConstraint();
	Super::OnDestroy();
}

void AConstraint::OnTick(float DeltaTime)
{
	Super::OnTick(DeltaTime);

	if (Constraint == nullptr && World != nullptr && World->WorldType == EWorldType::Game)
	{
		TryCreateConstraint();
	}

	if (Constraint != nullptr && NeedsRuntimeTick())
	{
		WakeBodies();
	}
}

void AConstraint::OnUiPropertyChange()
{
	Super::OnUiPropertyChange();

	DestroyConstraint();
	bNeedsTicking = true;
	TryCreateConstraint();
}

bool AConstraint::NeedsRuntimeTick() const
{
	return bKeepBodiesAwake;
}

btTypedConstraint* AConstraint::CreateConstraint()
{
	Log::Error("[Constraint] Base AConstraint cannot create a Bullet constraint directly: %s", Name.c_str());
	return nullptr;
}

bool AConstraint::TryCreateConstraint()
{
	if (Constraint != nullptr)
	{
		bNeedsTicking = NeedsRuntimeTick();
		return true;
	}

	if (!BodyA || !BodyB || GetBodyARigidbody() == nullptr || GetBodyBRigidbody() == nullptr)
	{
		bNeedsTicking = true;
		return false;
	}

	Constraint = CreateConstraint();
	if (Constraint == nullptr)
	{
		bNeedsTicking = true;
		return false;
	}

	Constraint->setBreakingImpulseThreshold(bCanBreak ? BreakingImpulseThreshold : SIMD_INFINITY);
	World->Physics.mWorld->addConstraint(Constraint, bDisableCollisionBetweenBodies);

	bNeedsTicking = NeedsRuntimeTick();
	if (bNeedsTicking)
	{
		WakeBodies();
	}

	Log::Message(
		"[%s] Created %s BodyA=%s BodyB=%s Breakable=%i CollisionDisabled=%i",
		GetTypeVirtual()->Name,
		Name.c_str(),
		BodyA->Name.c_str(),
		BodyB->Name.c_str(),
		bCanBreak ? 1 : 0,
		bDisableCollisionBetweenBodies ? 1 : 0);

	return true;
}

void AConstraint::DestroyConstraint()
{
	if (Constraint == nullptr)
	{
		return;
	}

	if (World != nullptr && World->Physics.mWorld != nullptr)
	{
		World->Physics.mWorld->removeConstraint(Constraint);
	}

	delete Constraint;
	Constraint = nullptr;
}

void AConstraint::WakeBodies()
{
	if (BodyA && BodyA->PhysicsBody != nullptr)
	{
		BodyA->PhysicsBody->Activate();
	}

	if (BodyB && BodyB->PhysicsBody != nullptr)
	{
		BodyB->PhysicsBody->Activate();
	}
}

btRigidBody* AConstraint::GetBodyARigidbody()
{
	return BodyA && BodyA->PhysicsBody != nullptr ? BodyA->PhysicsBody->mRigidbody : nullptr;
}

btRigidBody* AConstraint::GetBodyBRigidbody()
{
	return BodyB && BodyB->PhysicsBody != nullptr ? BodyB->PhysicsBody->mRigidbody : nullptr;
}

static Matrix GetBodyWorldFrameWithoutScale(AMeshInstance* Body)
{
	Transform BodyFrame = Body->GetWorldTransform();
	BodyFrame.Scale = Vector3(1.0f);
	BodyFrame.Update();
	return BodyFrame.GetMatrix();
}

Matrix AConstraint::BuildBodyLocalFrameMatrix(AMeshInstance* Body, const Matrix& ConstraintFrameWorld)
{
	Transform BodyFrame = Body->GetWorldTransform();
	BodyFrame.Scale = Vector3(1.0f);
	BodyFrame.Update();

	return BodyFrame.GetWorldToLocalMatrix() * ConstraintFrameWorld;
}

Matrix AConstraint::BuildConstraintFrameWorld(EConstraintAxis AuthorAxis, EConstraintAxis BulletAxisColumn)
{
	return BuildConstraintFrameWorldFromAxis(GetWorldTransform(), AuthorAxis, BulletAxisColumn);
}

bool AConstraint::UpdateCachedFrames(EConstraintAxis AuthorAxis, EConstraintAxis BulletAxisColumn)
{
	bHasCachedFrames = false;

	if (!BodyA || !BodyB)
	{
		return false;
	}

	const Matrix FrameWorld = BuildConstraintFrameWorld(AuthorAxis, BulletAxisColumn);
	BodyAFrameLocal = BuildBodyLocalFrameMatrix(BodyA.Thing, FrameWorld);
	BodyBFrameLocal = BuildBodyLocalFrameMatrix(BodyB.Thing, FrameWorld);
	bHasCachedFrames = true;
	return true;
}

bool AConstraint::HasCachedFrames() const
{
	return bHasCachedFrames && BodyA.Thing != nullptr && BodyB.Thing != nullptr;
}

Matrix AConstraint::GetBodyAFrameWorld() const
{
	if (!HasCachedFrames())
	{
		return Matrix(1.0f);
	}

	return GetBodyWorldFrameWithoutScale(BodyA.Thing) * BodyAFrameLocal;
}

Matrix AConstraint::GetBodyBFrameWorld() const
{
	if (!HasCachedFrames())
	{
		return Matrix(1.0f);
	}

	return GetBodyWorldFrameWithoutScale(BodyB.Thing) * BodyBFrameLocal;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// AFixedConstraint

btTypedConstraint* AFixedConstraint::CreateConstraint()
{
	if (!UpdateCachedFrames(EConstraintAxis::X, EConstraintAxis::Z))
	{
		return nullptr;
	}

	return new btFixedConstraint(
		*GetBodyARigidbody(),
		*GetBodyBRigidbody(),
		ToBulletTransform(BodyAFrameLocal),
		ToBulletTransform(BodyBFrameLocal));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// APointConstraint

btTypedConstraint* APointConstraint::CreateConstraint()
{
	if (!UpdateCachedFrames(EConstraintAxis::X, EConstraintAxis::Z))
	{
		return nullptr;
	}

	const btTransform FrameA = ToBulletTransform(BodyAFrameLocal);
	const btTransform FrameB = ToBulletTransform(BodyBFrameLocal);
	return new btPoint2PointConstraint(*GetBodyARigidbody(), *GetBodyBRigidbody(), FrameA.getOrigin(), FrameB.getOrigin());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// ASliderConstraint

btTypedConstraint* ASliderConstraint::CreateConstraint()
{
	// Bullet slider uses column 0 of the constraint frame as the linear axis.
	if (!UpdateCachedFrames(Axis, EConstraintAxis::X))
	{
		return nullptr;
	}

	btSliderConstraint* Slider = new btSliderConstraint(
		*GetBodyARigidbody(),
		*GetBodyBRigidbody(),
		ToBulletTransform(BodyAFrameLocal),
		ToBulletTransform(BodyBFrameLocal),
		true);

	Slider->setLowerLinLimit(LowerLinearLimit);
	Slider->setUpperLinLimit(UpperLinearLimit);
	Slider->setLowerAngLimit(Math::Radians(LowerAngularLimitDegrees));
	Slider->setUpperAngLimit(Math::Radians(UpperAngularLimitDegrees));

	Slider->setPoweredLinMotor(bEnableLinearMotor);
	Slider->setTargetLinMotorVelocity(LinearMotorTargetVelocity);
	Slider->setMaxLinMotorForce(MaxLinearMotorForce);

	Slider->setPoweredAngMotor(bEnableAngularMotor);
	Slider->setTargetAngMotorVelocity(Math::Radians(AngularMotorTargetVelocityDegrees));
	Slider->setMaxAngMotorForce(MaxAngularMotorForce);

	if ((bEnableLinearMotor && MaxLinearMotorForce <= 0.0f) || (bEnableAngularMotor && MaxAngularMotorForce <= 0.0f))
	{
		Log::Warning("[SliderConstraint] Motor enabled with non-positive max force on %s", Name.c_str());
	}

	return Slider;
}

bool ASliderConstraint::NeedsRuntimeTick() const
{
	return bKeepBodiesAwake || bEnableLinearMotor || bEnableAngularMotor;
}

Matrix ASliderConstraint::GetFrameWorld()
{
	// Bullet slider uses column 0 of the constraint frame as the linear axis.
	return BuildConstraintFrameWorld(Axis, EConstraintAxis::X);
}

Vector3 ASliderConstraint::GetAxisWorld()
{
	return GetFrameAxis(GetFrameWorld(), EConstraintAxis::X);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// AHingeConstraint

btTypedConstraint* AHingeConstraint::CreateConstraint()
{
	// Bullet hinge uses column 2 of the constraint frame as the hinge axis.
	if (!UpdateCachedFrames(Axis, EConstraintAxis::Z))
	{
		return nullptr;
	}

	btHingeConstraint* Hinge = new btHingeConstraint(
		*GetBodyARigidbody(),
		*GetBodyBRigidbody(),
		ToBulletTransform(BodyAFrameLocal),
		ToBulletTransform(BodyBFrameLocal));

	if (bUseLimits)
	{
		Hinge->setLimit(
			Math::Radians(LowerLimitDegrees),
			Math::Radians(UpperLimitDegrees),
			LimitSoftness,
			LimitBiasFactor,
			LimitRelaxationFactor);
	}

	Hinge->enableAngularMotor(
		bEnableMotor,
		Math::Radians(MotorTargetVelocityDegrees),
		MaxMotorImpulse);

	if (bEnableMotor)
	{
		if (MaxMotorImpulse <= 0.0f)
		{
			Log::Warning("[HingeConstraint] Motor is enabled but MaxMotorImpulse is %.3f", MaxMotorImpulse);
		}

		if (!BodyA->PhysicsBody->IsDynamic() && !BodyB->PhysicsBody->IsDynamic())
		{
			Log::Warning("[HingeConstraint] Motor is enabled but neither body is dynamic");
		}
	}

	return Hinge;
}

bool AHingeConstraint::NeedsRuntimeTick() const
{
	return bKeepBodiesAwake || bEnableMotor;
}

Matrix AHingeConstraint::GetFrameWorld()
{
	// Bullet hinge uses column 2 of the constraint frame as the hinge axis.
	return BuildConstraintFrameWorld(Axis, EConstraintAxis::Z);
}

Matrix AHingeConstraint::GetBodyAFrameWorldForCurrentPose()
{
	const Matrix FrameWorld = GetFrameWorld();

	if (BodyA.Thing == nullptr)
	{
		return FrameWorld;
	}

	const Matrix BodyAFrameLocalForCurrentPose = BuildBodyLocalFrameMatrix(BodyA.Thing, FrameWorld);
	return GetBodyWorldFrameWithoutScale(BodyA.Thing) * BodyAFrameLocalForCurrentPose;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// AConeTwistConstraint

btTypedConstraint* AConeTwistConstraint::CreateConstraint()
{
	// Bullet cone-twist uses column 0 of the constraint frame as the twist axis.
	if (!UpdateCachedFrames(TwistAxis, EConstraintAxis::X))
	{
		return nullptr;
	}

	btConeTwistConstraint* ConeTwist = new btConeTwistConstraint(
		*GetBodyARigidbody(),
		*GetBodyBRigidbody(),
		ToBulletTransform(BodyAFrameLocal),
		ToBulletTransform(BodyBFrameLocal));

	ConeTwist->setLimit(
		Math::Radians(SwingSpan1Degrees),
		Math::Radians(SwingSpan2Degrees),
		Math::Radians(TwistSpanDegrees),
		LimitSoftness,
		LimitBiasFactor,
		LimitRelaxationFactor);

	return ConeTwist;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Reflection

}

using namespace Columbus;

CREFLECT_ENUM_BEGIN(EConstraintAxis, "")
	CREFLECT_ENUM_FIELD(EConstraintAxis::X, 0)
	CREFLECT_ENUM_FIELD(EConstraintAxis::Y, 1)
	CREFLECT_ENUM_FIELD(EConstraintAxis::Z, 2)
CREFLECT_ENUM_END()

///////////////////////////////////////////////////////////////////////////////////////////////////
// Invisible Walls

CREFLECT_DEFINE_VIRTUAL(AInvisibleWall);
CREFLECT_STRUCT_BEGIN(AInvisibleWall, "")
	CREFLECT_STRUCT_DEFINE_INSTANTIATE()
	CREFLECT_STRUCT_FIELD(bool, bEnabled, "")
	CREFLECT_STRUCT_FIELD(HCollisionShapeDesc, CollisionShape, "")
	CREFLECT_STRUCT_FIELD(float, Restitution, "SliderMin(0) SliderMax(2)")
	CREFLECT_STRUCT_FIELD(float, Friction, "SliderMin(0) SliderMax(2)")
	CREFLECT_STRUCT_FIELD(float, RollingFriction, "SliderMin(0) SliderMax(1)")
CREFLECT_STRUCT_END()

///////////////////////////////////////////////////////////////////////////////////////////////////
// Constraints

CREFLECT_DEFINE_VIRTUAL(AConstraint);
CREFLECT_STRUCT_BEGIN(AConstraint, "")
	CREFLECT_STRUCT_FIELD_THINGREF(AMeshInstance, BodyA, "")
	CREFLECT_STRUCT_FIELD_THINGREF(AMeshInstance, BodyB, "")
	CREFLECT_STRUCT_FIELD(bool,  bDisableCollisionBetweenBodies, "")
	CREFLECT_STRUCT_FIELD(bool,  bCanBreak, "")
	CREFLECT_STRUCT_FIELD(float, BreakingImpulseThreshold, "")
	CREFLECT_STRUCT_FIELD(bool,  bKeepBodiesAwake, "")
CREFLECT_STRUCT_END()

CREFLECT_DEFINE_VIRTUAL(AFixedConstraint);
CREFLECT_STRUCT_BEGIN(AFixedConstraint, "")
	CREFLECT_STRUCT_DEFINE_INSTANTIATE()
CREFLECT_STRUCT_END()

CREFLECT_DEFINE_VIRTUAL(APointConstraint);
CREFLECT_STRUCT_BEGIN(APointConstraint, "")
	CREFLECT_STRUCT_DEFINE_INSTANTIATE()
CREFLECT_STRUCT_END()

CREFLECT_DEFINE_VIRTUAL(ASliderConstraint);
CREFLECT_STRUCT_BEGIN(ASliderConstraint, "")
	CREFLECT_STRUCT_DEFINE_INSTANTIATE()
	CREFLECT_STRUCT_FIELD(EConstraintAxis, Axis, "")
	CREFLECT_STRUCT_FIELD(float, LowerLinearLimit, "")
	CREFLECT_STRUCT_FIELD(float, UpperLinearLimit, "")
	CREFLECT_STRUCT_FIELD(float, LowerAngularLimitDegrees, "")
	CREFLECT_STRUCT_FIELD(float, UpperAngularLimitDegrees, "")
	CREFLECT_STRUCT_FIELD(bool,  bEnableLinearMotor, "")
	CREFLECT_STRUCT_FIELD(float, LinearMotorTargetVelocity, "")
	CREFLECT_STRUCT_FIELD(float, MaxLinearMotorForce, "")
	CREFLECT_STRUCT_FIELD(bool,  bEnableAngularMotor, "")
	CREFLECT_STRUCT_FIELD(float, AngularMotorTargetVelocityDegrees, "")
	CREFLECT_STRUCT_FIELD(float, MaxAngularMotorForce, "")
CREFLECT_STRUCT_END()

CREFLECT_DEFINE_VIRTUAL(AHingeConstraint);
CREFLECT_STRUCT_BEGIN(AHingeConstraint, "")
	CREFLECT_STRUCT_DEFINE_INSTANTIATE()
	CREFLECT_STRUCT_FIELD(EConstraintAxis, Axis, "")
	CREFLECT_STRUCT_FIELD(bool,  bUseLimits, "")
	CREFLECT_STRUCT_FIELD(float, LowerLimitDegrees, "")
	CREFLECT_STRUCT_FIELD(float, UpperLimitDegrees, "")
	CREFLECT_STRUCT_FIELD(float, LimitSoftness, "SliderMin(0) SliderMax(1)")
	CREFLECT_STRUCT_FIELD(float, LimitBiasFactor, "SliderMin(0) SliderMax(1)")
	CREFLECT_STRUCT_FIELD(float, LimitRelaxationFactor, "SliderMin(0) SliderMax(2)")
	CREFLECT_STRUCT_FIELD(bool,  bEnableMotor, "")
	CREFLECT_STRUCT_FIELD(float, MotorTargetVelocityDegrees, "")
	CREFLECT_STRUCT_FIELD(float, MaxMotorImpulse, "")
CREFLECT_STRUCT_END()

CREFLECT_DEFINE_VIRTUAL(AConeTwistConstraint);
CREFLECT_STRUCT_BEGIN(AConeTwistConstraint, "")
	CREFLECT_STRUCT_DEFINE_INSTANTIATE()
	CREFLECT_STRUCT_FIELD(EConstraintAxis, TwistAxis, "")
	CREFLECT_STRUCT_FIELD(float, SwingSpan1Degrees, "")
	CREFLECT_STRUCT_FIELD(float, SwingSpan2Degrees, "")
	CREFLECT_STRUCT_FIELD(float, TwistSpanDegrees, "")
	CREFLECT_STRUCT_FIELD(float, LimitSoftness, "SliderMin(0) SliderMax(1)")
	CREFLECT_STRUCT_FIELD(float, LimitBiasFactor, "SliderMin(0) SliderMax(1)")
	CREFLECT_STRUCT_FIELD(float, LimitRelaxationFactor, "SliderMin(0) SliderMax(2)")
CREFLECT_STRUCT_END()
