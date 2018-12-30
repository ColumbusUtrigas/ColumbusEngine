#pragma once

#include <Graphics/Material.h>
#include <Graphics/Particles/ParticleModule.h>
#include <Graphics/Particles/Emit/ParticleModuleEmit.h>
#include <Graphics/Particles/Required/ParticleModuleRequired.h>

namespace Columbus
{

	class ParticleEffect
	{
	private:
		Material* mMaterial = nullptr;
		Vector3 mPos = Vector3(0, 0, 0);
	public:
		std::vector<ParticleModule*> Modules;
		ParticleModuleEmit* Emit;
		ParticleModuleRequired* Required;
	public:
		ParticleEffect();
		ParticleEffect(std::string aFile);
		ParticleEffect(std::string aFile, Material* aMaterial);

		void AddModule(ParticleModule* Module);
		ParticleModule* GetModule(ParticleModule::Type Type) const;

		void setMaterial(const Material* aMaterial);
		void setPos(const Vector3 aPos);
		void addPos(const Vector3 aPos);

		Material* getMaterial() const;
		Vector3 getPos() const;

		bool saveToXML(std::string aFile) const;
		bool loadFromXML(std::string aFile);
		bool load(std::string aFile);

		~ParticleEffect();
	};

}



