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

	//////////////////////////////////////////////////////////////////////////////
	Camera::Camera()
	{

	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	void Camera::setPos(const Vector3 aPos)
	{
		mPos = static_cast<Vector3>(aPos);
	}
	//////////////////////////////////////////////////////////////////////////////
	void Camera::addPos(const Vector3 aPos)
	{
		mPos += aPos;
	}
	//////////////////////////////////////////////////////////////////////////////
	Vector3 Camera::getPos() const
	{
		return mPos;
	}
	//////////////////////////////////////////////////////////////////////////////
	void Camera::setRot(const Vector3 aRot)
	{
		mRot = static_cast<Vector3>(aRot);
	}
	//////////////////////////////////////////////////////////////////////////////
	void Camera::addRot(const Vector3 aRot)
	{
		mRot += aRot;
	}
	//////////////////////////////////////////////////////////////////////////////
	Vector3 Camera::getRot() const
	{
		return mRot;
	}
	//////////////////////////////////////////////////////////////////////////////
	void Camera::setTarget(const Vector3 aTarget)
	{
		mTarget = static_cast<Vector3>(aTarget);
	}
	//////////////////////////////////////////////////////////////////////////////
	void Camera::addTarget(const Vector3 aTarget)
	{
		mTarget += aTarget;
	}
	//////////////////////////////////////////////////////////////////////////////
	Vector3 Camera::getTarget() const
	{
		return mTarget;
	}
	//////////////////////////////////////////////////////////////////////////////
	Vector3 Camera::direction() const
	{
		return mCameraDirection;
	}
	//////////////////////////////////////////////////////////////////////////////
	Vector3 Camera::right() const
	{
		return -mCameraRight;
	}
	//////////////////////////////////////////////////////////////////////////////
	Vector3 Camera::up() const
	{
		return mCameraUp;
	}
	//////////////////////////////////////////////////////////////////////////////
	void Camera::update()
	{
		while (mRot.x >= 360.0f || mRot.x <= -360.0f)
		{
			mRot.x -= 360.0f * Math::Sign(mRot.x);
		}

		while (mRot.y >= 360.0f || mRot.y <= -360.0f)
		{
			mRot.y -= 360.0f * Math::Sign(mRot.y);
		}

		while (mRot.z >= 360.0f || mRot.z <= -360.0f)
		{
			mRot.z -= 360.0f * Math::Sign(mRot.z);
		}

		/*if (mRot.x >= 360 || mRot.x <= -360) mRot.x = 0.0;
		if (mRot.y >= 360 || mRot.y <= -360) mRot.y = 0.0;
		if (mRot.z >= 360 || mRot.z <= -360) mRot.z = 0.0;*/

		//Vector3 Front;
		mCameraDirection.z = Math::Cos(Math::Radians(mRot.x)) * Math::Cos(Math::Radians(mRot.y));
		mCameraDirection.y = Math::Sin(Math::Radians(mRot.x));
		mCameraDirection.x = Math::Cos(Math::Radians(mRot.x)) * Math::Sin(Math::Radians(mRot.y));
		mCameraDirection = -mCameraDirection.normalize();
		//mCameraDirection = -Front.normalize();

		Vector3 up = Vector3(0.0f, 1.0f, 0.0f);
		mCameraRight = Vector3::cross(up, mCameraDirection).normalize();

		mCameraUp = Vector3::cross(mCameraDirection, mCameraRight);
		
		//mViewMatrix = glm::lookAt(mPos.toGLM(), mCameraDirection.toGLM() + mPos.toGLM(), mCameraUp.toGLM());
		ViewMatrix.LookAt(mPos, mCameraRight, mCameraDirection, mCameraUp);

		preTargeted = false;
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	void Camera::perspective(float FOV, float Aspect, float Near, float Far)
	{
		//mProjectionMatrix.perspective(FOV, Aspect, Near, Far);
		//mProjectionMatrix = glm::perspective(FOV, Aspect, Near, Far);
		ProjectionMatrix.Perspective(FOV, Aspect, Near, Far);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	Matrix Camera::getProjectionMatrix() const
	{
		return ProjectionMatrix;
	}
	//////////////////////////////////////////////////////////////////////////////
	Matrix Camera::getViewMatrix() const
	{
		return ViewMatrix;
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	Camera::~Camera()
	{

	}

}
