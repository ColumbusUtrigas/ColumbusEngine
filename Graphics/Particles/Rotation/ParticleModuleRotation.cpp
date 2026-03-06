#include <Graphics/Particles/Rotation/ParticleModuleRotation.h>
#include <Core/Platform.h>

#if defined(PLATFORM_WINDOWS)
	#include <intrin.h>
#elif defined(PLATFORM_LINUX)
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

	void ParticleModuleRotation::Update(ParticleContainer& Container, float DeltaTime) const
	{
	#if USE_SIMD_EXTENSIONS
		__m128 DeltaTimes = _mm_set_ps1(DeltaTime);

		for (size_t i = 0, id = 0; i < (Container.Count + 3) / 4; i++, id += 4)
		{
			__m128 RotationVelocities = _mm_load_ps(&Container.RotationVelocities[id]);
			__m128 Delta = _mm_mul_ps(RotationVelocities, DeltaTimes);

			__m128 Rotations = _mm_load_ps(&Container.Rotations[id]);
			Rotations = _mm_add_ps(Rotations, Delta);
			_mm_store_ps(&Container.Rotations[id], Rotations);
		}
	#else
		for (size_t i = 0; i < Container.Count; i++)
		{
			Container.Rotations[i] += Container.RotationVelocities[i] * DeltaTime;
		}
	#endif
	}

}


