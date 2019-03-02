#include <Graphics/Particles/Rotation/ParticleModuleRotation.h>
#include <Core/Platform/Platform.h>

#if defined(_WIN32) || defined(_WIN64)
	#include <intrin.h>
#elif defined(__unix)
	#include <x86intrin.h>
#endif

namespace Columbus
{

	void ParticleModuleRotation::Spawn(const ParticleContainer& Container, size_t Spawn) const
	{
		for (size_t i = Container.Count; i < Container.Count + Spawn; i++)
		{
			Container.Rotations[i] = Random::Range(Min, Max);
			Container.RotationVelocities[i] = Random::Range(MinVelocity, MaxVelocity);
		}
	}

	void ParticleModuleRotation::Update(ParticleContainer& Container) const
	{
	#if USE_SIMD_EXTENSIONS
		for (size_t i = 0, id = 0; i < (Container.Count + 3) / 4; i++, id += 4)
		{
			__m128 RotationVelocities = _mm_load_ps(&Container.RotationVelocities[id]);
			__m128 Ages = _mm_load_ps(&Container.Ages[id]);
			__m128 Rotations = _mm_mul_ps(RotationVelocities, Ages);
			_mm_store_ps(&Container.Rotations[id], Rotations);
		}
	#else
		for (size_t i = 0; i < Container.Count; i++)
		{
			Container.Rotations[i] = Container.RotationVelocities[i] * Container.Ages[i];
		}
	#endif
	}

}


