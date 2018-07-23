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
		Type GetType() const override { return Type::Emit; }

		~ParticleModuleEmit() override {}
	};

}










