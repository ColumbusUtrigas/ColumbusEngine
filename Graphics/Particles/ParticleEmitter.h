#pragma once

#include <Graphics/Buffer.h>
#include <Graphics/Camera.h>
#include <Graphics/Particles/ParticleEffect.h>
#include <Graphics/Particles/Particle.h>

#include <Math/Vector2.h>
#include <Math/Vector3.h>
#include <Math/Vector4.h>

namespace Columbus
{

	class ParticleEmitter
	{
	private:
		ParticleEffect* Effect = nullptr;

		Particle* Particles = nullptr;

		Buffer* VerticesBuffer = nullptr;
		Buffer* UVBuffer = nullptr;

		Buffer* Positions = nullptr;
		Buffer* Times = nullptr;
		Buffer* Colors = nullptr;
		Buffer* Sizes = nullptr;
		
		Vector3* VertData = nullptr;
		Vector2* UVData = nullptr;

		Vector3* PositionData = nullptr;
		Vector2* TimeData = nullptr;
		Vector4* ColorData = nullptr;
		Vector3* SizeData = nullptr;
		
		void UpdateMainBuffers();

		void SetBuffers();
		void SetUniforms();
		
		float Life = 0.0f;
		float Timer = 0.0f;

		uint32 ActiveCount = 0;
		uint32 MaxCount = 0;

		Camera ObjectCamera;
	public:
		ParticleEmitter(ParticleEffect* InEffect);

		void SetParticleEffect(ParticleEffect* InEffect);
		ParticleEffect* GetParticleEffect() const;

		void Update(float TimeTick);
		void Render();

		void SetCamera(const Camera& Cam) { ObjectCamera = Cam; }

		~ParticleEmitter();
	};

}
