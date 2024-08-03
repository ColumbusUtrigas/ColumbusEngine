#pragma once

#include <Math/Matrix.h>
#include <Math/Vector3.h>

namespace Columbus
{

	class Camera
	{
	private:
		Vector3 CameraDirection = Vector3(0, 0, -1);
		Vector3 CameraRight = Vector3(1, 0, 0);
		Vector3 CameraUp = Vector3(0, 1, 0);

		Matrix ProjectionMatrix;
		Matrix ViewMatrix;
		Matrix ViewProjection;

		bool PreTargeted = false;
		Vector3 Target = Vector3(0, 0, 4);

		Vector2 Jittering;

		float FovY = 45.0f;
		float AspectRatio = 1.0f;
	public:
		Vector3 Pos = Vector3(0, 0, 5);
		Vector3 Rot = Vector3(0, 0, 0);

		bool  EnableDoF = false;
		float FStop = 1.4f;
		float FocusDistance = 10.0f;
		float SensorSize = 0.1f;

		bool  EnableGrain = false;
		float GrainScale  = 0.01f;
		float GrainAmount = 0.7f;

		// Chromatic aberration
		bool  EnableChromaticAberration = false;
		float ChromaticAberrationAmount = 0.5f;

		bool  EnableVignette = false;
		float Vignette = 0.5f;

		void SetTarget(const Vector3& NewTarget)
		{
			Target = NewTarget;
			PreTargeted = true;
		}
	public:
		Camera();

		void Update();

		Vector3 Direction() const;
		Vector3 Right() const;
		Vector3 Up() const;

		void Perspective(float FOV, float Aspect, float Near, float Far);
		void Ortho(float Left, float Right, float Bottom, float Top, float Near, float Far);

		float GetFovX() const;
		float GetFovY() const;
		float GetAspect() const;

		float GetFocalLength() const;
		float GetApertureDiameter() const;

		const Matrix& GetViewProjection() const;
		const Matrix& GetProjectionMatrix() const;
		const Matrix& GetViewMatrix() const;

		Vector2 GetJittering() const { return Jittering; }

		void ApplyProjectionJitter(float X, float Y);

		// having viewspace normalised coordinates, shoot a ray into the world and return it's direction
		Vector3 CalcRayByNdc(const Vector2& NDC) const;
		
		~Camera();
	};

}
