#pragma once

#include <Graphics/Material.h>
#include <Graphics/Particles/ParticleModule.h>
#include <Graphics/Particles/Emit/ParticleModuleEmit.h>
#include <Graphics/Particles/Required/ParticleModuleRequired.h>
#include <Core/Containers/Vector.h>
#include <Core/SmartPointer.h>

namespace Columbus
{

	class ParticleEffect
	{
	public:
		Vector<SmartPointer<ParticleModule>> Modules;
		ParticleModuleEmit Emit;
		ParticleModuleRequired Required;

		Material Material;
		Vector3 Position;
	public:
		ParticleEffect();

		void AddModule(ParticleModule* Module);
		ParticleModule* GetModule(ParticleModule::Type Type) const;

		bool Load(const char* FileName);

		~ParticleEffect();
	};

}



