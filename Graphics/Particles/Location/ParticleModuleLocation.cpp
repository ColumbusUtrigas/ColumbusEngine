#include <Graphics/Particles/Location/ParticleModuleLocation.h>
#include <cmath>

namespace Columbus
{

	void ParticleModuleLocation::SpawnBox(const ParticleContainer& Container, size_t Spawn, const Vector3& Base) const
	{
		if (EmitFromShell)
		{
			for (size_t i = Container.Count; i < Container.Count + Spawn; i++)
			{
				switch (rand() % 3)
				{
				case 0:
					Container.Positions[i].X = Size.X * 0.5f * ((rand() % 2) ? -1 : 1);
					Container.Positions[i].Y = Random::Range(-Size.Y * 0.5f, Size.Y * 0.5f);
					Container.Positions[i].Z = Random::Range(-Size.Z * 0.5f, Size.Z * 0.5f);
					break;
				case 1:
					Container.Positions[i].X = Random::Range(-Size.X * 0.5f, Size.X * 0.5f);
					Container.Positions[i].Y = Size.Y * 0.5f * ((rand() % 2) ? -1 : 1);
					Container.Positions[i].Z = Random::Range(-Size.Z * 0.5f, Size.Z * 0.5f);
					break;
				case 2:
					Container.Positions[i].X = Random::Range(-Size.X * 0.5f, Size.X * 0.5f);
					Container.Positions[i].Y = Random::Range(-Size.Y * 0.5f, Size.Y * 0.5f);
					Container.Positions[i].Z = Size.Z * 0.5f * ((rand() % 2) ? -1 : 1);
					break;
				}

				Container.Positions[i] += Base;
			}
		}
		else
		{
			for (size_t i = Container.Count; i < Container.Count + Spawn; i++)
			{
				Container.Positions[i].X = Random::Range(-Size.X * 0.5f, Size.X * 0.5f);
				Container.Positions[i].Y = Random::Range(-Size.Y * 0.5f, Size.Y * 0.5f);
				Container.Positions[i].Z = Random::Range(-Size.Z * 0.5f, Size.Z * 0.5f);
				Container.Positions[i] += Base;
			}
		}
	}

	void ParticleModuleLocation::SpawnCircle(const ParticleContainer& Container, size_t Spawn, const Vector3& Base) const
	{
		for (size_t i = Container.Count; i < Container.Count + Spawn; i++)
		{
			float Ang = Random::Range(0.0f, 6.283185306f);
			float Rad = EmitFromShell ? Radius : Random::Range(0.0f, Radius);

			Container.Positions[i].X = Rad * cosf(Ang);
			Container.Positions[i].Y = 0.0;
			Container.Positions[i].Z = Rad * sinf(Ang);
			Container.Positions[i] += Base;
		}
	}

	void ParticleModuleLocation::SpawnSphere(const ParticleContainer& Container, size_t Spawn, const Vector3& Base) const
	{
		for (size_t i = Container.Count; i < Container.Count + Spawn; i++)
		{
			float Rad = EmitFromShell ? Radius : Random::Range(0.0f, Radius);
			float Phi = Random::Range(0.0f, 6.283185306f);
			float Tht = Random::Range(0.0f, 3.141592653f);

			Container.Positions[i].X = Rad * cosf(Phi) * sinf(Tht);
			Container.Positions[i].Y = Rad * sinf(Phi) * sinf(Tht);
			Container.Positions[i].Z = Rad * cosf(Tht);
			Container.Positions[i] += Base;
		}
	}

	void ParticleModuleLocation::Spawn(const ParticleContainer& Container, size_t Spawn, const Vector3& Base) const
	{
		switch (Shape)
		{
		case SpawnShape::Point:  for (size_t i = Container.Count; i < Container.Count + Spawn; i++) Container.Positions[i] = Base; break;
		case SpawnShape::Box:    SpawnBox(Container, Spawn, Base); break;
		case SpawnShape::Circle: SpawnCircle(Container, Spawn, Base); break;
		case SpawnShape::Sphere: SpawnSphere(Container, Spawn, Base); break;
		}
	}

}


