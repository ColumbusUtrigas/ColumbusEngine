#pragma once

#include <Graphics/Particles/ParticleContainer.h>
#include <Math/InterpolationCurve.h>
#include <Math/Vector4.h>

namespace Columbus
{

	class ParticleModuleColor
	{
	public:
		enum class UpdateMode
		{
			Initial,
			OverLife
		};

		Vector4 Min;
		Vector4 Max;
		InterpolationCurve<Vector4> Curve;

		UpdateMode Mode = UpdateMode::Initial;
	public:
		ParticleModuleColor() : Min(1), Max(1) {}

		void Spawn(const ParticleContainer& Container, size_t Spawn) const
		{
			for (size_t i = Container.Count; i < Container.Count + Spawn; i++)
			{
				Container.Colors[i] = Vector4::Random(Min, Max);
			}
		}

		void Update(ParticleContainer& Container) const
		{
			if (Mode == UpdateMode::OverLife)
			{
				for (size_t i = 0; i < Container.Count; i++)
				{
					Container.Colors[i] = Curve.Interpolate(Container.Percents[i]);
				}
			}
		}

		~ParticleModuleColor() {}
	};

}


