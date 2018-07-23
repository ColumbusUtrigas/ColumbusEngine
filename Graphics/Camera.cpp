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

	
	Camera::Camera()
	{

	}
	
	void Camera::setPos(const Vector3 aPos)
	{
		mPos = static_cast<Vector3>(aPos);
	}
	
	void Camera::addPos(const Vector3 aPos)
	{
		mPos += aPos;
	}
	
	Vector3 Camera::getPos() const
	{
		return mPos;
	}
	
	void Camera::setRot(const Vector3 aRot)
	{
		mRot = static_cast<Vector3>(aRot);
	}
	
	void Camera::addRot(const Vector3 aRot)
	{
		mRot += aRot;
	}
	
	Vector3 Camera::getRot() const
	{
		return mRot;
	}
	
	void Camera::setTarget(const Vector3 aTarget)
	{
		mTarget = static_cast<Vector3>(aTarget);
	}
	
	void Camera::addTarget(const Vector3 aTarget)
	{
		mTarget += aTarget;
	}
	
	Vector3 Camera::getTarget() const
	{
		return mTarget;
	}
	
	Vector3 Camera::direction() const
	{
		return mCameraDirection;
	}

	Vector3 Camera::right() const
	{
		return -mCameraRight;
	}
	
	Vector3 Camera::up() const
	{
		return mCameraUp;
	}
	
	void Camera::update()
	{
		while (mRot.X >= 360.0f || mRot.X <= -360.0f)
		{
			mRot.X -= 360.0f * Math::Sign(mRot.X);
		}

		while (mRot.Y >= 360.0f || mRot.Y <= -360.0f)
		{
			mRot.Y -= 360.0f * Math::Sign(mRot.Y);
		}

		while (mRot.Z >= 360.0f || mRot.Z <= -360.0f)
		{
			mRot.Z -= 360.0f * Math::Sign(mRot.Z);
		}

		mCameraDirection.Z = Math::Cos(Math::Radians(mRot.X)) * Math::Cos(Math::Radians(mRot.Y));
		mCameraDirection.Y = Math::Sin(Math::Radians(mRot.X));
		mCameraDirection.X = Math::Cos(Math::Radians(mRot.X)) * Math::Sin(Math::Radians(mRot.Y));
		mCameraDirection = -mCameraDirection.Normalize();

		Vector3 up = Vector3(0.0f, 1.0f, 0.0f);
		mCameraRight = Vector3::Cross(up, mCameraDirection).Normalize();

		mCameraUp = Vector3::Cross(mCameraDirection, mCameraRight);

		ViewMatrix.LookAt(mPos, mCameraDirection + mPos, mCameraUp);

		preTargeted = false;
	}
	
	void Camera::perspective(float FOV, float Aspect, float Near, float Far)
	{
		ProjectionMatrix.Perspective(FOV, Aspect, Near, Far);
	}
	
	Matrix Camera::getProjectionMatrix() const
	{
		return ProjectionMatrix;
	}
	
	Matrix Camera::getViewMatrix() const
	{
		return ViewMatrix;
	}
	
	Camera::~Camera()
	{

	}

}
