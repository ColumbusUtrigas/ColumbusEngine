#pragma once

#include <Math/Vector3.h>
#include <Math/Matrix.h>

#include <glm/gtc/quaternion.hpp>

namespace Columbus
{

	class Transform
	{
	private:
		Vector3 Position;
		Vector3 Rotation;
		Vector3 Scale = Vector3(1, 1, 1);
		Matrix ModelMatrix;
	public:
		glm::quat RotationQuaternion;

		Transform();
		Transform(Vector3 Pos);
		Transform(Vector3 Pos, Vector3 Rot);
		Transform(Vector3 Pos, Vector3 Rot, Vector3 Scale);

		void SetPos(Vector3 Pos);
		void AddPos(Vector3 Pos);
		Vector3 GetPos() const;

		void SetRot(Vector3 Rot);
		void SetRot(glm::quat InRotation);
		void AddRot(Vector3 Rot);
		Vector3 GetRot() const;

		void SetScale(Vector3 Scale);
		void AddScale(Vector3 Scale);
		Vector3 GetScale() const;

		void Update();

		void SetMatrix(Matrix InMatrix);
		const Matrix& GetMatrix() const;

		~Transform();
	};

}


