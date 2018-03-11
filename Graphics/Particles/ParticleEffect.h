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
#include <Graphics/Particles/Emit/ParticleModuleEmit.h>
#include <Graphics/Particles/Lifetime/ParticleModuleLifetime.h>
#include <Graphics/Particles/Velocity/ParticleModuleVelocity.h>
#include <Graphics/Particles/Velocity/ParticleModuleVelocityBase.h>
#include <Graphics/Particles/SubUV/ParticleModuleSubUV.h>
#include <Graphics/Particles/SubUV/ParticleModuleSubUVBase.h>
#include <System/Log.h>
#include <System/Serializer.h>
#include <json.hpp>

namespace Columbus
{

	struct ParticleLocation
	{
		bool EmitFromShell = false;
		ParticleShape Shape;
		float Radius;
		Vector3 Size;
	};

	struct ParticleInitialAcceleration
	{
		Vector3 Min = Vector3(0, 0.1, 0);
		Vector3 Max = Vector3(0, 0.1, 0);
	};

	struct ParticleInitialSize
	{
		Vector3 Min = Vector3(1, 1, 1);
		Vector3 Max = Vector3(1, 1, 1);
	};

	struct ParticleSizeOverLife
	{
		bool Active = true;

		Vector3 MinStart = Vector3(1, 1, 1);
		Vector3 MaxStart = Vector3(1, 1, 1);
		Vector3 MinFinal = Vector3(1, 1, 1);
		Vector3 MaxFinal = Vector3(1, 1, 1);
	};

	struct ParticleInitialColor
	{
		Vector4 Min = Vector4(1, 1, 1, 1);
		Vector4 Max = Vector4(1, 1, 1, 1);
	};

	struct ParticleColorOverLife
	{
		bool Active = true;

		Vector4 MinStart = Vector4(1, 1, 1, 1);
		Vector4 MaxStart = Vector4(1, 1, 1, 1);
		Vector4 MinFinal = Vector4(1, 1, 1, 1);
		Vector4 MaxFinal = Vector4(1, 1, 1, 1);
	};

	class ParticleEffect
	{
	private:
		Material* mMaterial = nullptr;
		Vector3 mPos = Vector3(0, 0, 0);

		std::vector<ParticleModule*> Modules;
	public:

		ParticleRequiredModule Required;
		ParticleLocation Location; //TODO
		ParticleRotationModule Rotation;

		ParticleInitialAcceleration InitialAcceleration; //TODO
		ParticleInitialSize InitialSize; //TODO
		ParticleSizeOverLife SizeOverLife; //TODO
		ParticleInitialColor InitialColor; //TODO
		ParticleColorOverLife ColorOverLife; //TODO
		ParticleNoiseModule Noise;
	public:
		ParticleEffect();
		ParticleEffect(std::string aFile);
		ParticleEffect(std::string aFile, Material* aMaterial);

		void AddModule(ParticleModule* Module);
		ParticleModule* GetModule(ParticleModuleType Type) const;

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
