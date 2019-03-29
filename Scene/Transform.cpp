#include <Scene/Transform.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Columbus
{

	Transform::Transform() :
		ModelMatrix(1.0f) {}

	Transform::Transform(Vector3 Pos) :
		Position(Pos),
		ModelMatrix(1.0f) {}

	Transform::Transform(Vector3 Pos, Vector3 Rot) :
		Position(Pos),
		Rotation(Rot),
		ModelMatrix(1.0f) {}

	Transform::Transform(Vector3 Pos, Vector3 Rot, Vector3 Scale) :
		Position(Pos),
		Rotation(Rot),
		Scale(Scale),
		ModelMatrix(1.0f) {}
	
	void Transform::Update()
	{
		RotationQuaternion = glm::quat(glm::vec3(Math::Radians(Rotation.X), Math::Radians(Rotation.Y), Math::Radians(Rotation.Z)));
		glm::mat4 RotationMatrix = glm::mat4_cast(RotationQuaternion);
		float* Value = glm::value_ptr(RotationMatrix);

		Matrix tRotationMatrix;
		memcpy(&tRotationMatrix.M[0][0], Value, 64);

		ModelMatrix.SetIdentity();
		ModelMatrix.Scale(Scale);
		//ModelMatrix.Rotate(Vector3(1, 0, 0), Rotation.X);
		//ModelMatrix.Rotate(Vector3(0, 1, 0), Rotation.Y);
		//ModelMatrix.Rotate(Vector3(0, 0, 1), Rotation.Z);
		ModelMatrix = ModelMatrix * tRotationMatrix;
		ModelMatrix.Translate(Position);
	}
	
	void Transform::SetMatrix(Matrix InMatrix)
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


