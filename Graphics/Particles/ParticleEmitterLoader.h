#pragma once

#include <Graphics/Particles/ParticleEmitterCPU.h>

namespace Columbus
{

	struct ParticleEmitterLoader
	{
		static bool Load(ParticleEmitterCPU& Particles, const char* FileName);
	};

}


