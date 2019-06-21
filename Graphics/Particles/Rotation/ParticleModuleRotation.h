#pragma once

#include <Graphics/Particles/ParticleContainer.h>

namespace Columbus
{

	class ParticleModuleRotation
	{
	public:
		float Min;
		float Max;
		float MinVelocity;
		float MaxVelocity;
	public:
		ParticleModuleRotation() : Min(0.0f), Max(0.0f), MinVelocity(0.0f), MaxVelocity(0.0f) {}

		void Spawn(const ParticleContainer& Container, size_t Spawn) const;
		void Update(ParticleContainer& Container) const;

		~ParticleModuleRotation() {}
	};

}


