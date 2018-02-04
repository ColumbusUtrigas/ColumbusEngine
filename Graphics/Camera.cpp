/************************************************
*                  Camera.cpp                   *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#include <Graphics/Camera.h>

namespace Columbus
{

	static C_Matrix4 PROJECTION_MATRIX;
	static C_Matrix4 VIEW_MATRIX;

	//////////////////////////////////////////////////////////////////////////////
	C_Matrix4& C_GetProjectionMatrix()
	{
		return PROJECTION_MATRIX;
	}
	//////////////////////////////////////////////////////////////////////////////
	C_Matrix4 C_GetViewMatrix()
	{
		return VIEW_MATRIX;
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_SetPerspective(float aFOV, float aAspect, float aN, float aF)
	{
		PROJECTION_MATRIX.perspective(aFOV, aAspect, aN, aF);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_SetOrtho(float aL, float aR, float aB, float aT, float aN, float aF)
	{
		PROJECTION_MATRIX = glm::ortho(aL, aR, aB, aT, aN, aF);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_SetCamera(C_Vector3 aEye, C_Vector3 aRef)
	{
		VIEW_MATRIX = glm::lookAt(aEye.toGLM(), aRef.toGLM(), glm::vec3(0, 1, 0));
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	C_Camera::C_Camera()
	{
		mPos = glm::vec3(0, 0, 5);
		mRot = glm::vec3(0, 0, 0);
		mTarget = glm::vec3(0, 0, 4);
		mCameraDirection = glm::vec3(0, 0, -1);
		mCameraRight = glm::vec3(1, 0, 0);
		mCameraUp = glm::vec3(0, 1, 0);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	void C_Camera::setPos(C_Vector3 aPos)
	{
		mPos = static_cast<C_Vector3>(aPos).toGLM();
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Camera::addPos(C_Vector3 aPos)
	{
		mPos += static_cast<C_Vector3>(aPos).toGLM();
	}
	//////////////////////////////////////////////////////////////////////////////
	C_Vector3 C_Camera::getPos() const
	{
		return mPos;
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Camera::setRot(C_Vector3 aRot)
	{
		mRot = aRot.toGLM();
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Camera::addRot(C_Vector3 aRot)
	{
		mRot += aRot.toGLM();
	}
	//////////////////////////////////////////////////////////////////////////////
	C_Vector3 C_Camera::getRot() const
	{
		return mRot;
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Camera::setTarget(C_Vector3 aTarget)
	{
		mTarget = aTarget.toGLM();
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Camera::addTarget(C_Vector3 aTarget)
	{
		mTarget += aTarget.toGLM();
	}
	//////////////////////////////////////////////////////////////////////////////
	C_Vector3 C_Camera::getTarget() const
	{
		return mTarget;
	}
	//////////////////////////////////////////////////////////////////////////////
	C_Vector3 C_Camera::direction() const
	{
		return mCameraDirection;
	}
	//////////////////////////////////////////////////////////////////////////////
	C_Vector3 C_Camera::right() const
	{
		return -mCameraRight;
	}
	//////////////////////////////////////////////////////////////////////////////
	C_Vector3 C_Camera::up() const
	{
		return mCameraUp;
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Camera::update()
	{
		/*glm::vec3 y = glm::vec3(0, 0, -1);
		C_Vector3 rot;
		rot.x = C_DegToRads(mRot.x);
		rot.y = C_DegToRads(mRot.y);
		rot.z = C_DegToRads(mRot.z);
		y = glm::rotateY(y, rot.y);
		y = glm::rotateX(y, rot.x);
		y = glm::rotateZ(y, rot.z);
		mTarget = y + mPos;*/

		if (mPos.x >= 360 || mPos.x <= -360) mPos.x = 0.0;
		if (mPos.y >= 360 || mPos.y <= -360) mPos.y = 0.0;
		if (mPos.z >= 360 || mPos.z <= -360) mPos.z = 0.0;

		if (mRot.x >= 360 || mRot.x <= -360) mRot.x = 0.0;
		if (mRot.y >= 360 || mRot.y <= -360) mRot.y = 0.0;
		if (mRot.z >= 360 || mRot.z <= -360) mRot.z = 0.0;

		glm::vec3 front;
		front.z = cos(glm::radians(mRot.x)) * cos(glm::radians(mRot.y));
		front.y = sin(glm::radians(mRot.x));
		front.x = cos(glm::radians(mRot.x)) * sin(glm::radians(mRot.y));
		mCameraDirection = glm::normalize(-front);

		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
		mCameraRight = glm::normalize(glm::cross(up, mCameraDirection));

		mCameraUp = glm::cross(mCameraDirection, mCameraRight);

		VIEW_MATRIX = glm::lookAt(mPos, mCameraDirection + mPos, mCameraUp);

		preTargeted = false;
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	C_Camera::~C_Camera()
	{

	}

}
