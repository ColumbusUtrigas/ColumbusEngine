#pragma once

#include <Graphics/Particles/Emit/ParticleModuleEmitBase.h>

namespace Columbus
{

	class ParticleModuleEmit : public ParticleModuleEmitBase
	{
	public:
		int Count;
		float EmitRate;

		ParticleModuleEmit() : Count(5), EmitRate(5.0f) {}
		/*
		* For determening module type
		*/
		ParticleModuleType GetType() const override { return E_PARTICLE_MODULE_EMIT; }

		~ParticleModuleEmit() override {}
	};

}










