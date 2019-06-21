#include <Graphics/Particles/Lifetime/ParticleModuleLifetime.h>

namespace Columbus
{

	void ParticleModuleLifetime::Spawn(const ParticleContainer& Container, size_t Spawn) const
	{
		for (size_t i = Container.Count; i < Container.Count + Spawn; i++)
		{
			Container.Lifetimes[i] = Random::Range(Min, Max);
		}
	}

}


