#pragma once

#include <Math/Vector3.h>
#include <Math/Matrix.h>
#include <Math/Quaternion.h>
#include <Core/ISerializable.h>

namespace Columbus
{

	class Transform : public ISerializable
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
		Quaternion Q;
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


