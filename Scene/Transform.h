#pragma once

#include <Math/Vector3.h>
#include <Math/Matrix.h>
#include <Math/Quaternion.h>
#include <Core/ISerializable.h>

namespace Columbus
{

	class Transform : public ISerializable
	{
	public:
		Vector3 Position;
		Quaternion Rotation;
		Vector3 Scale;
		Matrix ModelMatrix;
	public:
		Transform(const Vector3& Pos = {0}, const Vector3& Rot = {0}, const Vector3& Scale = {1});

		void Update();

		void SetMatrix(const Matrix& InMatrix);
		const Matrix& GetMatrix() const;

		virtual void Serialize(JSON& J) const final override;
		virtual void Deserialize(JSON& J) final override;

		~Transform();
	};

}


