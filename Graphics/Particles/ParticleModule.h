#pragma once

#include <Math/Vector3.h>
#include <System/Random.h>
#include <Graphics/Particles/Particle.h>

namespace Columbus
{
	/*
	* Shape im which new particle spawning
	*/
	enum ParticleShape
	{
		E_PARTICLE_SHAPE_CIRCLE,
		E_PARTICLE_SHAPE_SPHERE,
		E_PARTICLE_SHAPE_BOX
	};
	/*
	* Shitcode, i'm so bad
	*/
	enum ParticleTransformation
	{
		E_PARTICLE_TRANSFORMATION_LOCAL,
		E_PARTICLE_TRANSFORMATION_WORLD
	};
	/*
	* Particles may sorting by distance from camera and may be rendered without sorting
	*/
	enum ParticleSortMode
	{
		E_PARTICLE_SORT_MODE_NONE,
		E_PARTICLE_SORT_MODE_DISTANCE
	};
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
		E_PARTICLE_MODULE_SUBUV
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

		~ParticleModule() {}
	};
	/*
	* Required module implementation
	* This module is base of particle emitter
	*/
	class ParticleRequiredModule : public ParticleModule
	{
	public:
		bool Visible = true;
		bool AdditiveBlending = false;
		bool Billboarding = true;
		ParticleTransformation Transformation = E_PARTICLE_TRANSFORMATION_WORLD;
		ParticleSortMode SortMode = E_PARTICLE_SORT_MODE_DISTANCE;
	};
	/*
	* Rotation module implementation
	*/
	class ParticleRotationModule : public ParticleModule
	{
	public:
		float Min;
		float Max;
		float MinVelocity;
		float MaxVelocity;

		ParticleRotationModule() :
			Min(0.0f),
			Max(0.0f),
			MinVelocity(0.0f),
			MaxVelocity(0.0f) {}

		/*
		* Initial rotation of new particle
		*/
		virtual float GetInitialRotation() const
		{
			return Random::range(Min, Max);
		}
		/*
		* Initial rotation velocity of new particle
		*/
		virtual float GetInitialVelocity() const
		{
			return Random::range(MinVelocity, MaxVelocity);
		}

		~ParticleRotationModule() {}
	};
	/*
	*
	*/
	class ParticleNoiseModule : public ParticleModule
	{
	public:
		float Strength;
		int Octaves;
		float Lacunarity;
		float Persistence;
		float Frequency;
		float Amplitude;

		ParticleNoiseModule() :
			Strength(1.0f),
			Octaves(1),
			Lacunarity(2.0f),
			Persistence(0.5f),
			Frequency(1.0f),
			Amplitude(1.0f) {}

		~ParticleNoiseModule() {}
	};

}














