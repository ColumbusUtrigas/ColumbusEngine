/************************************************
*              ParticleEffect.h                 *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#pragma once

#include <Graphics/Material.h>

#include <Graphics/Particles/ParticleModule.h>
#include <Graphics/Particles/Acceleration/ParticleModuleAcceleration.h>
#include <Graphics/Particles/Acceleration/ParticleModuleAccelerationBase.h>
#include <Graphics/Particles/Color/ParticleModuleColor.h>
#include <Graphics/Particles/Color/ParticleModuleColorBase.h>
#include <Graphics/Particles/Color/ParticleModuleColorOverLife.h>
#include <Graphics/Particles/Emit/ParticleModuleEmit.h>
#include <Graphics/Particles/Emit/ParticleModuleEmitBase.h>
#include <Graphics/Particles/Location/ParticleModuleLocationCircle.h>
#include <Graphics/Particles/Location/ParticleModuleLocationBase.h>
#include <Graphics/Particles/Location/ParticleModuleLocationBox.h>
#include <Graphics/Particles/Location/ParticleModuleLocationSphere.h>
#include <Graphics/Particles/Lifetime/ParticleModuleLifetime.h>
#include <Graphics/Particles/Lifetime/ParticleModuleLifetimeBase.h>
#include <Graphics/Particles/Noise/ParticleModuleNoise.h>
#include <Graphics/Particles/Noise/ParticleModuleNoiseBase.h>
#include <Graphics/Particles/Required/ParticleModuleRequired.h>
#include <Graphics/Particles/Required/ParticleModuleRequiredBase.h>
#include <Graphics/Particles/Rotation/ParticleModuleRotation.h>
#include <Graphics/Particles/Rotation/ParticleModuleRotationBase.h>
#include <Graphics/Particles/Size/ParticleModuleSize.h>
#include <Graphics/Particles/Size/ParticleModuleSizeBase.h>
#include <Graphics/Particles/Size/ParticleModuleSizeOverLife.h>
#include <Graphics/Particles/Velocity/ParticleModuleVelocity.h>
#include <Graphics/Particles/Velocity/ParticleModuleVelocityBase.h>
#include <Graphics/Particles/SubUV/ParticleModuleSubUV.h>
#include <Graphics/Particles/SubUV/ParticleModuleSubUVBase.h>
#include <System/Log.h>
#include <System/Serializer.h>
#include <json.hpp>

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



