#pragma once

#include <Graphics/Particles/ParticleContainer.h>
#include <Math/InterpolationCurve.h>
#include <Math/Vector3.h>

namespace Columbus
{

	class ParticleModuleSize
	{
	public:
		enum class UpdateMode
		{
			Initial,
			OverLife
		};

		InterpolationCurve<Vector3> Curve;
		Vector3 Min;
		Vector3 Max;

		UpdateMode Mode;
	public:
		ParticleModuleSize() : Min(1), Max(1) {}

		void Spawn(const ParticleContainer& Container, size_t Spawn) const
		{
			if (Mode == UpdateMode::Initial)
			{
				for (size_t i = Container.Count; i < Container.Count + Spawn; i++)
				{
					Container.Sizes[i] = Vector3::Random(Min, Max);
				}
			}
		}

		void Update(ParticleContainer& Container) const
		{
			if (Mode == UpdateMode::OverLife)
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


