#pragma once

#include <Graphics/Particles/ParticleContainer.h>
#include <Math/Vector3.h>

namespace Columbus
{

	class ParticleModuleVelocity
	{ 
	public:
		Vector3 Min;
		Vector3 Max;
	public:
		ParticleModuleVelocity() : Min(-1), Max(1) {}

		void Spawn(const ParticleContainer& Container, size_t Spawn)
		{
			for (size_t i = Container.Count; i < Container.Count + Spawn; i++)
			{
				Container.Velocities[i] = Vector3::Random(Min, Max);
			}
		}

		~ParticleModuleVelocity() {}
	};

}


