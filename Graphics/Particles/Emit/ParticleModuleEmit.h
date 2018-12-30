#pragma once

#include <Graphics/Particles/ParticleModule.h>

namespace Columbus
{

	class ParticleModuleEmit : public ParticleModule
	{
	public:
		int Count;
		float EmitRate;
	public:
		ParticleModuleEmit() : Count(5), EmitRate(5.0f) {}
		/*
		* For determening module type
		*/
		Type GetType() const override { return Type::Emit; }

		~ParticleModuleEmit() override {}
	};

}










