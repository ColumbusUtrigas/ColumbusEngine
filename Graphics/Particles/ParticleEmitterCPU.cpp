#include <Graphics/Particles/ParticleEmitterCPU.h>
#include <Core/Platform/Platform.h>
#include <Profiling/Profiling.h>

#if defined(PLATFORM_WINDOWS)
	#include <intrin.h>
#elif defined(PLATFORM_LINUX)
	#include <x86intrin.h>
#endif

namespace Columbus
{

	static bool SortYoungFirstPredicate(const ParticleContainer& Container, size_t A, size_t B)
	{
		return Container.Lifetimes[A] < Container.Lifetimes[B];
	}

	static bool SortOldFirstPredicate(const ParticleContainer& Container, size_t A, size_t B)
	{
		return Container.Lifetimes[A] > Container.Lifetimes[B];
	}

	static bool SortNearestFirstPredicate(const ParticleContainer& Container, size_t A, size_t B)
	{
		return Container.Distances[A] < Container.Distances[B];
	}

	void ParticleEmitterCPU::Update(float TimeTick)
	{
		PROFILE_CPU(ProfileModule::Particles);

		if (MaxParticles > PreviousMaxParticles)
		{
			PreviousMaxParticles = MaxParticles;
			Particles.Allocate(MaxParticles);
		}

		SpawnParticles(TimeTick);
		UpdateParticles(TimeTick);
		RemoveOldParticles();
		SortParticles();
	}

	void ParticleEmitterCPU::SpawnParticles(float TimeTick)
	{
		if (Emit)
		{
			Time += TimeTick;
			float FireTime = 1.0f / EmitRate;

			size_t NeedToSpawn = 0;

			while (Time >= FireTime && Particles.Count < MaxParticles)
			{
				Time -= FireTime;
				Particles.Ages[Particles.Count + NeedToSpawn] = Time;
				NeedToSpawn += Particles.Count + NeedToSpawn < MaxParticles ? 1 : 0;
			}

			ModuleLifetime.Spawn(Particles, NeedToSpawn);
			ModuleLocation.Spawn(Particles, NeedToSpawn, Position);
			ModuleVelocity.Spawn(Particles, NeedToSpawn);
			ModuleRotation.Spawn(Particles, NeedToSpawn);
			ModuleColor.Spawn(Particles, NeedToSpawn);
			ModuleSize.Spawn(Particles, NeedToSpawn);
			ModuleSubUV.Spawn(Particles, NeedToSpawn);
			Particles.Count += NeedToSpawn;
		}
	}

	void ParticleEmitterCPU::UpdateParticles(float TimeTick)
	{
		// Iterate over active particles and update them

		#if USE_SIMD_EXTENSIONS
			__m128 FourTimeTicks = _mm_load1_ps(&TimeTick);
			for (size_t i = 0; i < (Particles.Count + 3) / 4; i++)
			{
				size_t id = i * 4;
				__m128 FourAges      = _mm_load_ps(&Particles.Ages[id]);
				__m128 FourLifetimes = _mm_load_ps(&Particles.Lifetimes[id]);
				FourAges = _mm_add_ps(FourAges, FourTimeTicks);
				__m128 FourPercents = _mm_div_ps(FourAges, FourLifetimes);
				_mm_store_ps(&Particles.Ages[id], FourAges);
				_mm_store_ps(&Particles.Percents[id], FourPercents);

				for (int j = 0; j < 4; j++)
				{
					Particles.Positions[id + j] += Particles.Velocities[id + j] * TimeTick;
					Particles.Distances[id + j] = Particles.Positions[id + j].LengthSquare(CameraPosition);
				}
			}
		#else //SIMD instructions disabled
			for (size_t i = 0; i < Particles.Count; i++)
			{
				Particles.Ages[i] += TimeTick;
				Particles.Positions[i] += Particles.Velocities[i] * TimeTick;
				Particles.Percents[i] = Particles.Ages[i] / Particles.Lifetimes[i];
				Particles.Distances[i] = Particles.Positions[i].LengthSquare(CameraPosition);
			}
		#endif

		ModuleRotation.Update(Particles);
		ModuleColor.Update(Particles);
		ModuleSize.Update(Particles);
		ModuleSubUV.Update(Particles);
		//ModuleNoise.Update(Particles, TimeTick);
	}

	void ParticleEmitterCPU::RemoveOldParticles()
	{
		// Iterate over 'active' particles and moves dead of them. R.I.P. (Rest In Particles)
		for (size_t i = 0; i < Particles.Count; i++)
		{
			if (Particles.Percents[i] > 1.0f)
			{
				Particles.Move(i--, --Particles.Count);
			}
		}
	}

	void ParticleEmitterCPU::SortParticles()
	{
		if (Sort != SortMode::None)
		{
			bool(*Predicates[4])(const ParticleContainer&, size_t, size_t);
			//Predicates[(size_t)SortMode::None];
			Predicates[(size_t)SortMode::YoungFirst] = SortYoungFirstPredicate;
			Predicates[(size_t)SortMode::OldFirst] = SortOldFirstPredicate;
			Predicates[(size_t)SortMode::NearestFirst] = SortNearestFirstPredicate;

			const auto Predicate = Predicates[(size_t)Sort];

			bool Sorted = false;
			for (size_t i = 0; i < Particles.Count && !Sorted; i++)
			{
				Sorted = true;

				for (size_t j = 1; j < Particles.Count; j++)
				{
					if (Predicate(Particles, j - 1, j))
					{
						Particles.Swap(j - 1, j);
						Sorted = false;
					}
				}
			}
		}
	}

}


