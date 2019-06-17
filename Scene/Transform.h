#pragma once

#include <Math/Vector3.h>
#include <Math/Matrix.h>

#include <glm/gtc/quaternion.hpp>

namespace Columbus
{

	class Transform
	{
	private:
		Vector3 LastPosition;
		Vector3 LastRotation;
		Vector3 LastScale;
	public:
		Vector3 Position;
		Vector3 Rotation;
		Vector3 Scale;
		Matrix ModelMatrix;

		glm::quat RotationQuaternion;
	public:
		Transform(const Vector3& Pos = {0}, const Vector3& Rot = {0}, const Vector3& Scale = {1});

		void Update();

		void SetMatrix(const Matrix& InMatrix);
		const Matrix& GetMatrix() const;

		~Transform();
	};

}


