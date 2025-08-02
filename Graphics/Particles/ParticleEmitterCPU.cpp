#include <Graphics/Particles/ParticleEmitterCPU.h>
#include <Profiling/Profiling.h>
#include <Core/Platform.h>

#if defined(PLATFORM_WINDOWS)
	#include <intrin.h>
#elif defined(PLATFORM_LINUX)
	#include <x86intrin.h>
#endif

DECLARE_COUNTING_PROFILING_COUNTER(CountingCounter_Particles_NumUpdates)
DECLARE_COUNTING_PROFILING_COUNTER(CountingCounter_Particles_TotalParticles)
DECLARE_CPU_PROFILING_COUNTER(Counter_Particles_UpdateTime)

IMPLEMENT_COUNTING_PROFILING_COUNTER("Num Updates", "Particles", CountingCounter_Particles_NumUpdates, true);
IMPLEMENT_COUNTING_PROFILING_COUNTER("Total Particles", "Particles", CountingCounter_Particles_TotalParticles, true);
IMPLEMENT_CPU_PROFILING_COUNTER("Total Time", "Particles", Counter_Particles_UpdateTime);

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

	void HParticleEmitterInstanceCPU::Update(float TimeTick)
	{
		if (!Settings.IsValid())
			return;

		PROFILE_CPU(Counter_Particles_UpdateTime);

		Particles.Allocate(Settings->MaxParticles);

		SpawnParticles(TimeTick);
		UpdateParticles(TimeTick);
		RemoveOldParticles();
		SortParticles();

		AddProfilingCount(CountingCounter_Particles_NumUpdates, 1);
		AddProfilingCount(CountingCounter_Particles_TotalParticles, Particles.Count);
	}

	void HParticleEmitterInstanceCPU::SpawnParticles(float TimeTick)
	{
		if (Settings->bEmit)
		{
			SimulationTime += TimeTick;
			float FireTime = 1.0f / Settings->EmitRate;

			size_t NeedToSpawn = 0;

			while (SimulationTime >= FireTime && Particles.Count < Settings->MaxParticles)
			{
				SimulationTime -= FireTime;
				Particles.Ages[Particles.Count + NeedToSpawn] = SimulationTime;
				NeedToSpawn += Particles.Count + NeedToSpawn < Settings->MaxParticles ? 1 : 0;
			}

			Settings->Lifetime.Spawn(Particles, NeedToSpawn);
			Settings->Location.Spawn(Particles, NeedToSpawn, CurrentPosition);
			Settings->Velocity.Spawn(Particles, NeedToSpawn);
			Settings->Rotation.Spawn(Particles, NeedToSpawn);
			Settings->Color.Spawn(Particles, NeedToSpawn);
			Settings->Size.Spawn(Particles, NeedToSpawn);
			Settings->SubUV.Spawn(Particles, NeedToSpawn);
			Particles.Count += NeedToSpawn;
		}
	}

	void HParticleEmitterInstanceCPU::UpdateParticles(float TimeTick)
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

				for (size_t j = 0; j < 4; j++)
				{
					Particles.Positions[id + j] += Particles.Velocities[id + j] * TimeTick;
					Particles.Distances[id + j] = Particles.Positions[id + j].DistanceSquare(CameraPosition);
				}
			}
		#else //SIMD instructions disabled
			for (size_t i = 0; i < Particles.Count; i++)
			{
				Particles.Ages[i] += TimeTick;
				Particles.Positions[i] += Particles.Velocities[i] * TimeTick;
				Particles.Percents[i] = Particles.Ages[i] / Particles.Lifetimes[i];
				Particles.Distances[i] = Particles.Positions[i].DistanceSquare(CameraPosition);
			}
		#endif

		Settings->Rotation.Update(Particles);
		Settings->Color.Update(Particles);
		Settings->Size.Update(Particles);
		Settings->SubUV.Update(Particles);
		//Settings->Noise.Update(Particles, TimeTick);
	}

	void HParticleEmitterInstanceCPU::RemoveOldParticles()
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

	void HParticleEmitterInstanceCPU::SortParticles()
	{
		if (Settings->Sort != EParticleSortMode::None)
		{
			bool(*Predicates[4])(const ParticleContainer&, size_t, size_t);
			//Predicates[(u32)EParticleSortMode::None];
			Predicates[(u32)EParticleSortMode::YoungFirst]   = SortYoungFirstPredicate;
			Predicates[(u32)EParticleSortMode::OldFirst]     = SortOldFirstPredicate;
			Predicates[(u32)EParticleSortMode::NearestFirst] = SortNearestFirstPredicate;

			const auto Predicate = Predicates[(u32)Settings->Sort];

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


////////////////////////////////////////////////////////////////////
// Reflection definitions

using namespace Columbus;

CREFLECT_ENUM_BEGIN(EParticleSpawnShape, "")
	CREFLECT_ENUM_FIELD(EParticleSpawnShape::Point, 0)
	CREFLECT_ENUM_FIELD(EParticleSpawnShape::Box, 1)
	CREFLECT_ENUM_FIELD(EParticleSpawnShape::Circle, 2)
	CREFLECT_ENUM_FIELD(EParticleSpawnShape::Sphere, 3)
CREFLECT_ENUM_END()

CREFLECT_ENUM_BEGIN(EParticleSubUVMode, "")
	CREFLECT_ENUM_FIELD(EParticleSubUVMode::Linear, 0)
	CREFLECT_ENUM_FIELD(EParticleSubUVMode::Random, 1)
CREFLECT_ENUM_END()

CREFLECT_ENUM_BEGIN(EParticleColorUpdateMode, "")
	CREFLECT_ENUM_FIELD(EParticleColorUpdateMode::Initial, 0)
	CREFLECT_ENUM_FIELD(EParticleColorUpdateMode::OverLife, 1)
CREFLECT_ENUM_END()

CREFLECT_ENUM_BEGIN(EParticleSizeUpdateMode, "")
	CREFLECT_ENUM_FIELD(EParticleSizeUpdateMode::Initial, 0)
	CREFLECT_ENUM_FIELD(EParticleSizeUpdateMode::OverLife, 1)
CREFLECT_ENUM_END()

CREFLECT_ENUM_BEGIN(EParticleBlendMode, "")
	CREFLECT_ENUM_FIELD(EParticleBlendMode::Default, 0)
	CREFLECT_ENUM_FIELD(EParticleBlendMode::Add, 1)
CREFLECT_ENUM_END()

CREFLECT_ENUM_BEGIN(EParticleBillboardMode, "")
	CREFLECT_ENUM_FIELD(EParticleBillboardMode::None, 0)
	CREFLECT_ENUM_FIELD(EParticleBillboardMode::Vertical, 1)
	CREFLECT_ENUM_FIELD(EParticleBillboardMode::Horizontal, 2)
	CREFLECT_ENUM_FIELD(EParticleBillboardMode::FaceToCamera, 3)
CREFLECT_ENUM_END()

CREFLECT_ENUM_BEGIN(EParticleTransformationMode, "")
	CREFLECT_ENUM_FIELD(EParticleTransformationMode::World, 0)
	CREFLECT_ENUM_FIELD(EParticleTransformationMode::Local, 1)
CREFLECT_ENUM_END()

CREFLECT_ENUM_BEGIN(EParticleSortMode, "")
	CREFLECT_ENUM_FIELD(EParticleSortMode::None, 0)
	CREFLECT_ENUM_FIELD(EParticleSortMode::YoungFirst, 1)
	CREFLECT_ENUM_FIELD(EParticleSortMode::OldFirst, 2)
	CREFLECT_ENUM_FIELD(EParticleSortMode::NearestFirst, 3)
CREFLECT_ENUM_END()

CREFLECT_STRUCT_BEGIN(ParticleModuleLifetime, "")
	CREFLECT_STRUCT_FIELD(float, Min, "")
	CREFLECT_STRUCT_FIELD(float, Max, "")
CREFLECT_STRUCT_END()

CREFLECT_STRUCT_BEGIN(ParticleModuleLocation, "")
	CREFLECT_STRUCT_FIELD(Vector3, Size, "")
	CREFLECT_STRUCT_FIELD(float, Radius, "")
	CREFLECT_STRUCT_FIELD(EParticleSpawnShape, Shape, "")
	CREFLECT_STRUCT_FIELD(bool, EmitFromShell, "")
CREFLECT_STRUCT_END()

CREFLECT_STRUCT_BEGIN(ParticleModuleVelocity, "")
	CREFLECT_STRUCT_FIELD(Vector3, Min, "")
	CREFLECT_STRUCT_FIELD(Vector3, Max, "")
CREFLECT_STRUCT_END()

CREFLECT_STRUCT_BEGIN(ParticleModuleRotation, "")
	CREFLECT_STRUCT_FIELD(float, Min, "")
	CREFLECT_STRUCT_FIELD(float, Max, "")
	CREFLECT_STRUCT_FIELD(float, MinVelocity, "")
	CREFLECT_STRUCT_FIELD(float, MaxVelocity, "")
CREFLECT_STRUCT_END()

CREFLECT_STRUCT_BEGIN(ParticleModuleColor, "")
	CREFLECT_STRUCT_FIELD(Vector4, Min, "Colour")
	CREFLECT_STRUCT_FIELD(Vector4, Max, "Colour")
	CREFLECT_STRUCT_FIELD(InterpolationCurveFloat4, Curve, "")
	CREFLECT_STRUCT_FIELD(EParticleColorUpdateMode, Mode, "")
CREFLECT_STRUCT_END()

CREFLECT_STRUCT_BEGIN(ParticleModuleSize, "")
	CREFLECT_STRUCT_FIELD(Vector3, Min, "")
	CREFLECT_STRUCT_FIELD(Vector3, Max, "")
	CREFLECT_STRUCT_FIELD(InterpolationCurve<Vector3>, Curve, "")
	CREFLECT_STRUCT_FIELD(EParticleSizeUpdateMode, Mode, "")
CREFLECT_STRUCT_END()

CREFLECT_STRUCT_BEGIN(ParticleModuleNoise, "")
	CREFLECT_STRUCT_FIELD(float, Strength, "")
	CREFLECT_STRUCT_FIELD(int, Octaves, "")
	CREFLECT_STRUCT_FIELD(float, Lacunarity, "")
	CREFLECT_STRUCT_FIELD(float, Persistence, "")
	CREFLECT_STRUCT_FIELD(float, Frequency, "")
	CREFLECT_STRUCT_FIELD(float, Amplitude, "")
CREFLECT_STRUCT_END()

CREFLECT_STRUCT_BEGIN(ParticleModuleSubUV, "")
	CREFLECT_STRUCT_FIELD(int, Horizontal, "")
	CREFLECT_STRUCT_FIELD(int, Vertical, "")
	CREFLECT_STRUCT_FIELD(float, Cycles, "")
	CREFLECT_STRUCT_FIELD(EParticleSubUVMode, Mode, "")
CREFLECT_STRUCT_END()

CREFLECT_STRUCT_BEGIN(HParticleEmitterSettings, "")
	CREFLECT_STRUCT_FIELD(int,   MaxParticles, "")
	CREFLECT_STRUCT_FIELD(bool,  bEmit, "")
	CREFLECT_STRUCT_FIELD(float, EmitRate, "")

	CREFLECT_STRUCT_FIELD_ASSETREF(Texture2, Texture, "")

	CREFLECT_STRUCT_FIELD(EParticleBlendMode, Blend, "")
	CREFLECT_STRUCT_FIELD(EParticleBillboardMode, Billboard, "")
	CREFLECT_STRUCT_FIELD(EParticleTransformationMode, Transformation, "")
	CREFLECT_STRUCT_FIELD(EParticleSortMode, Sort, "")

	CREFLECT_STRUCT_FIELD(ParticleModuleLifetime, Lifetime, "")
	CREFLECT_STRUCT_FIELD(ParticleModuleLocation, Location, "")
	CREFLECT_STRUCT_FIELD(ParticleModuleVelocity, Velocity, "")
	CREFLECT_STRUCT_FIELD(ParticleModuleRotation, Rotation, "")
	CREFLECT_STRUCT_FIELD(ParticleModuleColor,    Color, "")
	CREFLECT_STRUCT_FIELD(ParticleModuleSize,     Size, "")
	CREFLECT_STRUCT_FIELD(ParticleModuleNoise,    Noise, "")
	CREFLECT_STRUCT_FIELD(ParticleModuleSubUV,    SubUV, "")
CREFLECT_STRUCT_END()
