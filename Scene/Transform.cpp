#include <Scene/Transform.h>
#include <Common/JSON/JSON.h>

namespace Columbus
{

	Transform::Transform(const Vector3& Pos, const Vector3& Rot, const Vector3& Scale) :
		LastPosition(Pos + Vector3{1}),
		LastRotation(Rot + Vector3{1}),
		LastScale(Scale + Vector3{1}),
		Position(Pos),
		Rotation(Rot),
		Scale(Scale),
		ModelMatrix(1.0f) {}
	
	void Transform::Update()
	{
		if (LastPosition != Position || LastRotation != Rotation || LastScale != Scale)
		{
			Q = Quaternion({ Math::Radians(Rotation.X),
			                 Math::Radians(Rotation.Y),
			                 Math::Radians(Rotation.Z) });

			ModelMatrix.SetIdentity();
			ModelMatrix.Scale(Scale);
			ModelMatrix = Q.ToMatrix() * ModelMatrix;
			ModelMatrix.Translate(Position);

			LastPosition = Position;
			LastRotation = Rotation;
			LastScale = Scale;
		}
	}
	
	void Transform::SetMatrix(const Matrix& InMatrix)
	{
		this->ModelMatrix = InMatrix;
	}
	
	const Matrix& Transform::GetMatrix() const
	{
		return ModelMatrix;
	}

	void Transform::Serialize(JSON& J) const
	{
		J["Position"] = Position;
		J["Rotation"] = Rotation;
		J["Scale"] = Scale;
	}

	void Transform::Deserialize(JSON& J)
	{
		Position = J["Position"].GetVector3<float>();
		Rotation = J["Rotation"].GetVector3<float>();
		Scale = J["Scale"].GetVector3<float>();
	}
	
	Transform::~Transform()
	{

	}

}


