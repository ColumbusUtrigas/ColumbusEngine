#include <Scene/Transform.h>
#include <Common/JSON/JSON.h>

namespace Columbus
{

	Transform::Transform(const Vector3& Pos, const Vector3& Rot, const Vector3& Scale) :
		Position(Pos),
		Rotation(Rot),
		Scale(Scale),
		ModelMatrix(1.0f) {}
	
	void Transform::Update()
	{
		//if (Dirty)
		{
			ModelMatrix.SetIdentity();
			ModelMatrix.Scale(Scale);
			ModelMatrix = Rotation.ToMatrix() * ModelMatrix;
			ModelMatrix.Translate(Position);
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
		Position = J["Position"];
		Rotation = J["Rotation"];
		Scale = J["Scale"];
	}

	Transform::~Transform()
	{

	}

}


