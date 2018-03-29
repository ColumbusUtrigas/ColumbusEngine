/************************************************
*                  Transform.h                  *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   12.11.2017                  *
*************************************************/
#pragma once

#include <Math/Vector3.h>
#include <Math/Matrix4.h>

namespace Columbus
{

	class Transform
	{
	private:
		Vector3 mPos;
		Vector3 mRot;
		Vector3 mScale = Vector3(1, 1, 1);
		Matrix4 mMatrix;
		Matrix4 mNormalMatrix;
	public:
		Transform();
		Transform(Vector3 aPos);
		Transform(Vector3 aPos, Vector3 aRot);
		Transform(Vector3 aPos, Vector3 aRot, Vector3 aScale);

		void setPos(Vector3 aPos);
		void addPos(Vector3 aPos);
		Vector3 getPos() const;

		void setRot(Vector3 aRot);
		void addRot(Vector3 aRot);
		Vector3 getRot() const;

		void setScale(Vector3 aScale);
		void addScale(Vector3 aScale);
		Vector3 getScale() const;

		void update();

		void setMatrix(Matrix4 aMatrix);
		Matrix4 getMatrix() const;
		Matrix4 getNormalMatrix() const;

		~Transform();
	};

}


