#pragma once

#include <Graphics/Particles/Required/ParticleModuleRequiredBase.h>

namespace Columbus
{

	/*
	* Shitcode, i'm so bad
	*/
	enum class ParticleTransformation
	{
		Local,
		World
	};
	/*
	* Particles may sorting by distance from camera and may be rendered without sorting
	*/
	enum class ParticleSortMode
	{
		None,
		Distance
	};

	class ParticleModuleRequired : public ParticleModuleRequiredBase
	{
	public:
		bool Visible;
		bool AdditiveBlending;
		bool Billboarding;
		ParticleTransformation Transformation;
		ParticleSortMode SortMode;
	public:
		ParticleModuleRequired() :
			Visible(true),
			AdditiveBlending(false),
			Billboarding(true),
			Transformation(ParticleTransformation::World),
			SortMode(ParticleSortMode::Distance) {}
		/*
		* For determening module type
		*/
		Type GetType() const override { return Type::Required; }
		/*
		* Set parameter for particle
		*/
		void Spawn(Particle& OutParticle) override
		{
			
		}

		~ParticleModuleRequired() override {}
	};

}







