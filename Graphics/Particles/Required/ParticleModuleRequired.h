#pragma once

#include <Graphics/Particles/Required/ParticleModuleRequiredBase.h>

namespace Columbus
{

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

	class ParticleModuleRequired : public ParticleModuleRequiredBase
	{
	public:
		bool Visible;
		bool AdditiveBlending;
		bool Billboarding;
		ParticleTransformation Transformation;
		ParticleSortMode SortMode;

		ParticleModuleRequired() :
			Visible(true),
			AdditiveBlending(false),
			Billboarding(true),
			Transformation(E_PARTICLE_TRANSFORMATION_WORLD),
			SortMode(E_PARTICLE_SORT_MODE_DISTANCE) {}
		/*
		* For determening module type
		*/
		ParticleModuleType GetType() const override { return E_PARTICLE_MODULE_REQUIRED; }
		/*
		* Set parameter for particle
		*/
		void Spawn(Particle& aParticle) override
		{
			
		}

		~ParticleModuleRequired() override {}
	};

}







