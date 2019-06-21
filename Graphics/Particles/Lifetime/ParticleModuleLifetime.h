#pragma once

#include <Graphics/Particles/ParticleContainer.h>

namespace Columbus
{

	class ParticleModuleLifetime
	{
	public:
		float Min;
		float Max;
	public:
		ParticleModuleLifetime() : Min(1.0f), Max(1.0f) {}

		void Spawn(const ParticleContainer& Container, size_t Spawn) const;

		~ParticleModuleLifetime() {}
	};

}


