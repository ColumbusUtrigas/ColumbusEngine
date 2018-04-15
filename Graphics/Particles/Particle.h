#pragma once

#include <Math/Vector3.h>
#include <Math/Vector4.h>

namespace Columbus
{

	class Particle
	{
	private:
		size_t i;
	public:
		Vector4 startColor = Vector4(1, 1, 1, 1);
		Vector4 finalColor = Vector4(1, 1, 1, 1);
		Vector4 color = Vector4(1, 1, 1, 1);

		Vector3 velocity = Vector3(0, 1, 0);
		Vector3 startPos = Vector3(0, 0, 0);
		Vector3 startEmitterPos = Vector3(0, 0, 0);
		Vector3 accel = Vector3(0, 0.1, 0);
		Vector3 pos;
		Vector3 NoiseModifier;

		Vector3 startSize = Vector3(1, 1, 1);
		Vector3 finalSize = Vector3(1, 1, 1);
		Vector3 size = Vector3(1, 1, 1);

		float rotation = 0.0;
		float rotationSpeed = 0.0;
		float age = 0.0;
		float TTL = 1.0;
		float cameraDistance = 1000.0;
		float noise[9];
		float percent;

		unsigned int frame = 0;

		void update(const float aTimeTick, const vec3 aCamera)
		{
			for (i = 0; i < 9; i++) noise[i] = fmodf(noise[i] + (0.625f * aTimeTick), 256);

			age += aTimeTick;

			pos = velocity * age + (accel * 0.5 * age * age);
			pos += startPos + startEmitterPos;
			pos += NoiseModifier;

			rotation += rotationSpeed * aTimeTick;
			cameraDistance = pow(aCamera.X - pos.X, 2) + pow(aCamera.Y - pos.Y, 2) + pow(aCamera.Z - pos.Z, 2);
			percent = age / TTL;
		}
	};

}






