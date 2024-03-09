#include <Graphics/Camera.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Columbus
{

	Camera::Camera() {}
	
	Vector3 Camera::Direction() const
	{
		return CameraDirection;
	}

	Vector3 Camera::Right() const
	{
		return -CameraRight;
	}
	
	Vector3 Camera::Up() const
	{
		return CameraUp;
	}
	
	void Camera::Update()
	{
		while (Rot.X >= 360.0f || Rot.X <= -360.0f) { Rot.X -= 360.0f * Math::Sign(Rot.X); }
		while (Rot.Y >= 360.0f || Rot.Y <= -360.0f) { Rot.Y -= 360.0f * Math::Sign(Rot.Y); }
		while (Rot.Z >= 360.0f || Rot.Z <= -360.0f) { Rot.Z -= 360.0f * Math::Sign(Rot.Z); }

		if (PreTargeted)
		{
			CameraDirection = Target - Pos;
		}
		else
		{
			CameraDirection.Z = Math::Cos(Math::Radians(Rot.X)) * Math::Cos(Math::Radians(Rot.Y));
			CameraDirection.Y = Math::Sin(Math::Radians(Rot.X));
			CameraDirection.X = Math::Cos(Math::Radians(Rot.X)) * Math::Sin(Math::Radians(Rot.Y));
			CameraDirection = -CameraDirection.Normalized();
		}

		Vector3 up = Vector3(0.0f, 1.0f, 0.0f);
		CameraRight = Vector3::Cross(up, CameraDirection).Normalize();
		CameraUp = Vector3::Cross(CameraDirection, CameraRight);

		ViewMatrix.LookAt(Pos, CameraDirection + Pos, CameraUp);
		ViewProjection = ViewMatrix * ProjectionMatrix;

		PreTargeted = false;
	}
	
	void Camera::Perspective(float FOV, float Aspect, float Near, float Far)
	{
		ProjectionMatrix.Perspective(FOV, Aspect, Near, Far);
	}

	void Camera::Ortho(float Left, float Right, float Bottom, float Top, float Near, float Far)
	{
		auto mat = glm::ortho(Left, Right, Bottom, Top, Near, Far);
		memcpy(&ProjectionMatrix.M[0][0], glm::value_ptr(mat), 64);
	}

	const Matrix& Camera::GetViewProjection() const
	{
		return ViewProjection;
	}
	
	const Matrix& Camera::GetProjectionMatrix() const
	{
		return ProjectionMatrix;
	}
	
	const Matrix& Camera::GetViewMatrix() const
	{
		return ViewMatrix;
	}

	Vector3 Camera::CalcRayByNdc(const Vector2& NDC) const
	{
		Vector4 MouseVector(NDC, -1, 1);
		Matrix InvVP = ViewProjection.GetInverted();
		return (MouseVector * InvVP).XYZ() - Pos;
	}
	
	Camera::~Camera() {}

}
