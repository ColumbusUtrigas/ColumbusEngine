#pragma once

#include <Math/Vector3.h>
#include <System/Random.h>
#include <Graphics/Particles/Particle.h>

namespace Columbus
{

	/*
	* If nedded to create particle spritesheet animation or random texture from sheet
	*/
	enum ParticleSubUVMode
	{
		E_PARTICLE_SUB_UV_MODE_LINEAR,
		E_PARTICLE_SUB_UV_MODE_RANDOM
	};

	enum ParticleModuleType
	{
		E_PARTICLE_MODULE,
		E_PARTICLE_MODULE_EMIT,
		E_PARTICLE_MODULE_LIFETIME,
		E_PARTICLE_MODULE_VELOCITY,
		E_PARTICLE_MODULE_SUBUV,
		E_PARTICLE_MODULE_REQUIRED,
		E_PARTICLE_MODULE_NOISE,
		E_PARTICLE_MODULE_LOCATION,
		E_PARTICLE_MODULE_ROTATION,
		E_PARTICLE_MODULE_ACCELERATION,
		E_PARTICLE_MODULE_SIZE,
		E_PARTICLE_MODULE_COLOR
	};

	/*
	*
	*/
	class ParticleModule
	{
	public:
		bool Active;
	public:
		ParticleModule() : Active(false) {}
		/*
		* For determening module type
		*/
		virtual ParticleModuleType GetType() const { return E_PARTICLE_MODULE; }
		/*
		* Set parameter for particle
		*/
		virtual void Spawn(Particle& aParticle) {}
		/*
		* Update particle parameter
		*/
		virtual void Update(Particle& aParticle) {}

		virtual ~ParticleModule() {}
	};

}














