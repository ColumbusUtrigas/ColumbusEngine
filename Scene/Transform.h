#pragma once

#include <Math/Vector3.h>
#include <Math/Matrix.h>
#include <Math/Quaternion.h>

namespace Columbus
{

	class Transform
	{
	public:
		Vector3 Position;
		Quaternion Rotation;
		Vector3 Scale;
		bool bDirty = true;

	private:
		Matrix ModelMatrix;
		Matrix WorldToLocalMatrix;
	public:
		Transform(const Vector3& Pos = {0}, const Vector3& Rot = {0}, const Vector3& Scale = {1});

		void Update();

		void SetFromMatrix(const Matrix& WorldMatrix);

		const Matrix& GetMatrix() const;
		const Matrix& GetWorldToLocalMatrix();
	};

}

CREFLECT_DECLARE_STRUCT(Columbus::Transform, 1, "EA17C079-E76A-49F3-946A-03F138C5100B");

