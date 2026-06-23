#include <Scene/Transform.h>

using namespace Columbus;

static bool TransformNearlyEqual(float A, float B, float Epsilon = 0.00001f)
{
	return fabsf(A - B) <= Epsilon;
}

static bool TransformNearlyEqual(const Vector3& A, const Vector3& B, float Epsilon = 0.00001f)
{
	return TransformNearlyEqual(A.X, B.X, Epsilon)
		&& TransformNearlyEqual(A.Y, B.Y, Epsilon)
		&& TransformNearlyEqual(A.Z, B.Z, Epsilon);
}

static bool TransformNearlyEqual(const Quaternion& A, const Quaternion& B, float Epsilon = 0.00001f)
{
	return TransformNearlyEqual(A.X, B.X, Epsilon)
		&& TransformNearlyEqual(A.Y, B.Y, Epsilon)
		&& TransformNearlyEqual(A.Z, B.Z, Epsilon)
		&& TransformNearlyEqual(A.W, B.W, Epsilon);
}

CREFLECT_STRUCT_BEGIN(Transform, "")
	CREFLECT_STRUCT_FIELD(Vector3, Position, "ColourChannels")
	CREFLECT_STRUCT_FIELD(Quaternion, Rotation, "ColourChannels")
	CREFLECT_STRUCT_FIELD(Vector3, Scale, "ColourChannels")
CREFLECT_STRUCT_END()

Transform::Transform(const Vector3& Pos, const Vector3& Rot, const Vector3& Scale) :
	Position(Pos),
	Rotation(Rot),
	Scale(Scale),
	ModelMatrix(1.0f)
{
	Update();
}

void Transform::Update()
{
	if (bHasExactMatrix
		&& TransformNearlyEqual(Position, MatrixPosition)
		&& TransformNearlyEqual(Rotation, MatrixRotation)
		&& TransformNearlyEqual(Scale, MatrixScale))
	{
		return;
	}

	ModelMatrix.SetIdentity();
	ModelMatrix.Scale(Scale);
	ModelMatrix = Rotation.ToMatrix() * ModelMatrix;
	ModelMatrix.Translate(Position);

	WorldToLocalMatrix = ModelMatrix.GetInverted();

	MatrixPosition = Position;
	MatrixRotation = Rotation;
	MatrixScale = Scale;
	bHasExactMatrix = false;
}

void Transform::SetFromMatrix(const Matrix& WorldMatrix)
{
	Vector3 Rot;
	WorldMatrix.DecomposeTransform(Position, Rot, Scale);
	Rotation = Quaternion(Rot);
	SetMatrix(WorldMatrix);
}

void Transform::SetMatrix(const Matrix& InMatrix)
{
	ModelMatrix = InMatrix;
	WorldToLocalMatrix = ModelMatrix.GetInverted();
	MatrixPosition = Position;
	MatrixRotation = Rotation;
	MatrixScale = Scale;
	bHasExactMatrix = true;
}

const Matrix& Transform::GetMatrix() const
{
	return ModelMatrix;
}

const Matrix& Transform::GetWorldToLocalMatrix() const
{
	return WorldToLocalMatrix;
}
