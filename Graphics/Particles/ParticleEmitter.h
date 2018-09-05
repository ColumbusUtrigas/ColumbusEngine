#pragma once

#include <cstdio>
#include <cstdlib>
#include <vector>
#include <ctime>
#include <algorithm>

#include <Graphics/Buffer.h>
#include <Graphics/Camera.h>
#include <Graphics/Particles/ParticleEffect.h>
#include <Graphics/Particles/Particle.h>
#include <Graphics/Light.h>

#include <Common/Noise/OctaveNoise.h>

#include <Math/Vector2.h>
#include <Math/Vector3.h>
#include <Math/Vector4.h>

#include <System/System.h>
#include <System/Timer.h>
#include <Core/Random.h>

namespace Columbus
{

	class ParticleEmitter
	{
	private:
		struct ColorKey
		{
			Vector4 Color = Vector4(1, 1, 1, 1);
			float Key = 0.0;
		};
	private:
		ParticleEffect* Effect = nullptr;

		std::vector<Particle> Particles;
		std::vector<ColorKey> ColorKeys;

		Buffer* VerticesBuffer = nullptr;
		Buffer* UVBuffer = nullptr;

		Buffer* Positions = nullptr;
		Buffer* Times = nullptr;
		Buffer* Colors = nullptr;
		Buffer* Sizes = nullptr;
		
		float* VertData = nullptr;
		float* UVData = nullptr;

		Vector3* PositionData = nullptr;
		Vector2* TimeData = nullptr;
		Vector4* ColorData = nullptr;
		Vector3* SizeData = nullptr;
		
		void UpdateMainBuffers();

		void SetBuffers();
		void SetUniforms();
		
		float Life = 0.0f;
		float Timer = 0.0f;
		uint32 ParticlesCount = 0;
		Camera ObjectCamera;
	public:
		ParticleEmitter(ParticleEffect* InEffect);

		void SetParticleEffect(ParticleEffect* InEffect);
		ParticleEffect* GetParticleEffect() const;

		void Update(float TimeTick);
		void Render();

		void SetCamera(const Camera& InCamera);

		~ParticleEmitter();
	};

}
