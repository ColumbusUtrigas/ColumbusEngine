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

	static Matrix4 PROJECTION_MATRIX;
	static Matrix4 VIEW_MATRIX;

	//////////////////////////////////////////////////////////////////////////////
	Matrix4 C_GetProjectionMatrix()
	{
		return PROJECTION_MATRIX;
	}
	//////////////////////////////////////////////////////////////////////////////
	Matrix4 C_GetViewMatrix()
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
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	C_Camera::C_Camera()
	{

	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	void C_Camera::setPos(const Vector3 aPos)
	{
		mPos = static_cast<Vector3>(aPos);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Camera::addPos(const Vector3 aPos)
	{
		mPos += aPos;
	}
	//////////////////////////////////////////////////////////////////////////////
	Vector3 C_Camera::getPos() const
	{
		return mPos;
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Camera::setRot(const Vector3 aRot)
	{
		mRot = static_cast<Vector3>(aRot);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Camera::addRot(const Vector3 aRot)
	{
		mRot += aRot;
	}
	//////////////////////////////////////////////////////////////////////////////
	Vector3 C_Camera::getRot() const
	{
		return mRot;
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Camera::setTarget(const Vector3 aTarget)
	{
		mTarget = static_cast<Vector3>(aTarget);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Camera::addTarget(const Vector3 aTarget)
	{
		mTarget += aTarget;
	}
	//////////////////////////////////////////////////////////////////////////////
	Vector3 C_Camera::getTarget() const
	{
		return mTarget;
	}
	//////////////////////////////////////////////////////////////////////////////
	Vector3 C_Camera::direction() const
	{
		return mCameraDirection;
	}
	//////////////////////////////////////////////////////////////////////////////
	Vector3 C_Camera::right() const
	{
		return -mCameraRight;
	}
	//////////////////////////////////////////////////////////////////////////////
	Vector3 C_Camera::up() const
	{
		return mCameraUp;
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Camera::update()
	{
		if (mPos.x >= 360 || mPos.x <= -360) mPos.x = 0.0;
		if (mPos.y >= 360 || mPos.y <= -360) mPos.y = 0.0;
		if (mPos.z >= 360 || mPos.z <= -360) mPos.z = 0.0;

		if (mRot.x >= 360 || mRot.x <= -360) mRot.x = 0.0;
		if (mRot.y >= 360 || mRot.y <= -360) mRot.y = 0.0;
		if (mRot.z >= 360 || mRot.z <= -360) mRot.z = 0.0;

		vec3 front;
		front.z = cos(Radians(mRot.x)) * cos(Radians(mRot.y));
		front.y = sin(Radians(mRot.x));
		front.x = cos(Radians(mRot.x)) * sin(Radians(mRot.y));
		mCameraDirection = -front.normalize();

		vec3 up = vec3(0.0f, 1.0f, 0.0f);
		mCameraRight = vec3::cross(up, mCameraDirection).normalize();

		mCameraUp = vec3::cross(mCameraDirection, mCameraRight);

		VIEW_MATRIX = glm::lookAt(mPos.toGLM(), mCameraDirection.toGLM() + mPos.toGLM(), mCameraUp.toGLM());
		//VIEW_MATRIX.lookAt(mPos, mCameraDirection + mPos, mCameraUp);

		preTargeted = false;
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	C_Camera::~C_Camera()
	{

	}

}
