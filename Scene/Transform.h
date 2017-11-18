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

	class C_Transform
	{
	private:
		C_Vector3 mPos;
		C_Vector3 mRot;
		C_Vector3 mScale = C_Vector3(1, 1, 1);
		C_Matrix4 mMatrix;
		C_Matrix4 mNormalMatrix;
	public:
		C_Transform();

		void setPos(C_Vector3 aPos);
		void addPos(C_Vector3 aPos);
		C_Vector3 getPos() const;

		void setRot(C_Vector3 aRot);
		void addRot(C_Vector3 aRot);
		C_Vector3 getRot() const;

		void setScale(C_Vector3 aScale);
		void addScale(C_Vector3 aScale);
		C_Vector3 getScale() const;

		void update();

		void setMatrix(C_Matrix4 aMatrix);
		C_Matrix4 getMatrix() const;
		C_Matrix4 getNormalMatrix() const;

		~C_Transform();
	};

}


