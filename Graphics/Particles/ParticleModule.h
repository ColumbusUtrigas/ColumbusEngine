#pragma once

#include <Math/Vector3.h>
#include <Core/Random.h>
#include <Graphics/Particles/Particle.h>

namespace Columbus
{

	class ParticleModule
	{
	public:
		bool Active;
	public:
		enum class Type
		{
			Module,
			Emit,
			Lifetime,
			Velocity,
			SubUV,
			Required,
			Noise,
			Location,
			Rotation,
			Acceleration,
			Size,
			Color
		};
	public:
		ParticleModule() : Active(false) {}
		/*
		* For determening module type
		*/
		virtual Type GetType() const { return Type::Module; }
		/*
		* Set parameter for particle
		*/
		virtual void Spawn(Particle& OutParticle) {}
		/*
		* Update particle parameter
		*/
		virtual void Update(Particle& OutParticle) {}

		virtual ~ParticleModule() {}
	};

}














