#include <Scene/Transform.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>

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
			RotationQuaternion = glm::quat(glm::vec3(Math::Radians(Rotation.X), Math::Radians(Rotation.Y), Math::Radians(Rotation.Z)));
			glm::mat4 RotationMatrix = glm::mat4_cast(RotationQuaternion);
			float* Value = glm::value_ptr(RotationMatrix);

			Matrix tRotationMatrix;
			memcpy(&tRotationMatrix.M[0][0], Value, 64);

			ModelMatrix.SetIdentity();
			ModelMatrix.Scale(Scale);
			ModelMatrix = ModelMatrix * tRotationMatrix;
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
	
	Transform::~Transform()
	{

	}

}


