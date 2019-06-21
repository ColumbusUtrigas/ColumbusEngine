#pragma once

#include <Graphics/Particles/ParticleContainer.h>
#include <Math/Vector3.h>

namespace Columbus
{

	class ParticleModuleLocation
	{
	public:
		enum class SpawnShape
		{
			Point,
			Box,
			Circle,
			Sphere
		};

		Vector3 Size = Vector3(1);
		float Radius = 1.0f;

		SpawnShape Shape = SpawnShape::Point;
		bool EmitFromShell = false;
	private:
		void SpawnBox(const ParticleContainer& Container, size_t Spawn, const Vector3& Base) const;
		void SpawnCircle(const ParticleContainer& Container, size_t Spawn, const Vector3& Base) const;
		void SpawnSphere(const ParticleContainer& Container, size_t Spawn, const Vector3& Base) const;
	public:
		void Spawn(const ParticleContainer& Container, size_t Spawn, const Vector3& Base) const;
	};

}


