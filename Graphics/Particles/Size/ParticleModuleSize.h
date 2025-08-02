#pragma once

#include <Graphics/Particles/ParticleContainer.h>
#include <Math/InterpolationCurve.h>
#include <Math/Vector3.h>

namespace Columbus
{

	enum class EParticleSizeUpdateMode
	{
		Initial,
		OverLife
	};

	class ParticleModuleSize
	{
	public:
		InterpolationCurve<Vector3> Curve;
		Vector3 Min;
		Vector3 Max;

		EParticleSizeUpdateMode Mode;
	public:
		ParticleModuleSize() : Min(1), Max(1), Mode(EParticleSizeUpdateMode::Initial) {}

		void Spawn(const ParticleContainer& Container, size_t Spawn) const
		{
			if (Mode == EParticleSizeUpdateMode::Initial)
			{
				for (size_t i = Container.Count; i < Container.Count + Spawn; i++)
				{
					Container.Sizes[i] = Vector3::Random(Min, Max);
				}
			}
		}

		void Update(ParticleContainer& Container) const
		{
			if (Mode == EParticleSizeUpdateMode::OverLife)
			{
				for (size_t i = 0; i < Container.Count; i++)
				{
					Container.Sizes[i] = Curve.Interpolate(Container.Percents[i]);
				}
			}
		}

		~ParticleModuleSize() {}
	};

}


