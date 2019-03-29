#pragma once

#include <Math/Vector3.h>
#include <Math/Matrix.h>

#include <glm/gtc/quaternion.hpp>

namespace Columbus
{

	class Transform
	{
	public:
		Vector3 Position;
		Vector3 Rotation;
		Vector3 Scale = Vector3(1, 1, 1);
		Matrix ModelMatrix;

		glm::quat RotationQuaternion;
	public:
		Transform();
		Transform(Vector3 Pos);
		Transform(Vector3 Pos, Vector3 Rot);
		Transform(Vector3 Pos, Vector3 Rot, Vector3 Scale);

		void Update();

		void SetMatrix(Matrix InMatrix);
		const Matrix& GetMatrix() const;

		~Transform();
	};

}


