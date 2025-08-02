#pragma once

#include <Graphics/Particles/ParticleContainer.h>
#include <Graphics/Camera.h>

#include <Graphics/Particles/Lifetime/ParticleModuleLifetime.h>
#include <Graphics/Particles/Location/ParticleModuleLocation.h>
#include <Graphics/Particles/Velocity/ParticleModuleVelocity.h>
#include <Graphics/Particles/Rotation/ParticleModuleRotation.h>
#include <Graphics/Particles/Color/ParticleModuleColor.h>
#include <Graphics/Particles/Size/ParticleModuleSize.h>
#include <Graphics/Particles/Noise/ParticleModuleNoise.h>
#include <Graphics/Particles/SubUV/ParticleModuleSubUV.h>

#include <Graphics/Core/Texture.h>

#include <Core/Asset.h>
#include <Core/Reflection.h>

namespace Columbus
{

	enum class EParticleBlendMode
	{
		Default,
		Add,
	};

	enum class EParticleBillboardMode
	{
		None,
		Vertical,
		Horizontal,
		FaceToCamera
	};

	enum class EParticleTransformationMode
	{
		World,
		Local
	};

	enum class EParticleSortMode
	{
		None,
		YoungFirst,
		OldFirst,
		NearestFirst
	};

	struct HParticleEmitterSettings
	{
		int   MaxParticles = 100;
		bool  bEmit = true;
		float EmitRate = 5.0f;

		AssetRef<Texture2> Texture;

		EParticleBlendMode          Blend          = EParticleBlendMode::Default;
		EParticleBillboardMode      Billboard      = EParticleBillboardMode::FaceToCamera;
		EParticleTransformationMode Transformation = EParticleTransformationMode::World;
		EParticleSortMode           Sort           = EParticleSortMode::YoungFirst;

		ParticleModuleLifetime Lifetime;
		ParticleModuleLocation Location;
		ParticleModuleVelocity Velocity;
		ParticleModuleRotation Rotation;
		ParticleModuleColor    Color;
		ParticleModuleSize     Size;
		ParticleModuleNoise    Noise;
		ParticleModuleSubUV    SubUV;
	};

	struct HParticleEmitterInstanceCPU
	{
	public: // DATA
		AssetRef<HParticleEmitterSettings> Settings;
		ParticleContainer                  Particles;

		float SimulationTime = 0.0;

		Vector3 CurrentPosition;
		Vector3 CameraPosition;

	public:
		void Update(float TimeTick);

	private:
		void SpawnParticles(float TimeTick);
		void UpdateParticles(float TimeTick);
		void RemoveOldParticles();
		void SortParticles();
	};

}

// Reflection declarations

CREFLECT_DECLARE_ENUM(Columbus::EParticleSpawnShape, "E6041A1E-EB0B-4D14-AB03-0F668ADBE9DD")
CREFLECT_DECLARE_ENUM(Columbus::EParticleSubUVMode, "7799FC4E-952E-4ED2-9772-900C1F1C924B")
CREFLECT_DECLARE_ENUM(Columbus::EParticleColorUpdateMode, "7AC42957-C3D1-430E-8937-117FEF922310")
CREFLECT_DECLARE_ENUM(Columbus::EParticleSizeUpdateMode, "26304010-17AF-4EEC-A396-ED6DB7D1448C")

CREFLECT_DECLARE_ENUM(Columbus::EParticleBlendMode, "AD8A3E7D-F2E5-4D8C-AE5D-EFE03B7882B3")
CREFLECT_DECLARE_ENUM(Columbus::EParticleBillboardMode, "9337CDEC-FA05-436D-A289-7758200835C9")
CREFLECT_DECLARE_ENUM(Columbus::EParticleTransformationMode, "5E7722AB-1910-4200-BC40-D56A1A5638D9")
CREFLECT_DECLARE_ENUM(Columbus::EParticleSortMode, "D5FD83F3-DC01-4E94-9749-FF17BFC5E9A4")

CREFLECT_DECLARE_STRUCT(Columbus::ParticleModuleLifetime, 1, "D586998F-6096-4A87-B03D-B8D6A8E7EB21")
CREFLECT_DECLARE_STRUCT(Columbus::ParticleModuleLocation, 1, "275793FC-56E4-49CA-B45E-3CF118F99673")
CREFLECT_DECLARE_STRUCT(Columbus::ParticleModuleVelocity, 1, "3A5586CB-565C-414C-BA26-00D1689DED5C")
CREFLECT_DECLARE_STRUCT(Columbus::ParticleModuleRotation, 1, "C119DAE1-76F1-46FF-B0E1-E43F61801A33")
CREFLECT_DECLARE_STRUCT(Columbus::ParticleModuleColor, 1, "339C2E43-E4A8-47DB-9390-15DC160AB296")
CREFLECT_DECLARE_STRUCT(Columbus::ParticleModuleSize, 1, "F7A0BB5E-254A-417B-B937-B3BDEABCE429")
CREFLECT_DECLARE_STRUCT(Columbus::ParticleModuleNoise, 1, "E11EDF0D-6408-4B4B-8ADB-DB1468829DC7")
CREFLECT_DECLARE_STRUCT(Columbus::ParticleModuleSubUV, 1, "24A1A9E4-47F9-4094-A2ED-AD29AB723A9B")
CREFLECT_DECLARE_STRUCT(Columbus::HParticleEmitterSettings, 1, "B40708D7-9427-4B6A-9E7B-E02485D56C77")
