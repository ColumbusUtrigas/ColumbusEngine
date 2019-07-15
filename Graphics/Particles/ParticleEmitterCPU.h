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

#include <Core/ISerializable.h>

namespace Columbus
{

	class ParticleEmitterCPU : public ISerializable 
	{
	public:
		enum class BlendMode
		{
			Default,
			Add,
			Subtract,
			Multiply
		};

		enum class BillboardMode
		{
			None,
			Vertical,
			Horizontal,
			FaceToCamera
		};

		enum class TransformationMode
		{
			World,
			Local
		};

		enum class SortMode
		{
			None,
			YoungFirst,
			OldFirst,
			NearestFirst
		};
	private:
		friend class ParticlesRenderer;
		float Time = 0.0;

		size_t PreviousMaxParticles = 0;
	public:
		uint32 MaxParticles = 0;
		float EmitRate = 0;
		bool Emit = true;
		bool Visible = true;

		Vector3 Position = Vector3(10, 5, 10);

		ParticleModuleLifetime ModuleLifetime;
		ParticleModuleLocation ModuleLocation;
		ParticleModuleVelocity ModuleVelocity;
		ParticleModuleRotation ModuleRotation;
		ParticleModuleColor ModuleColor;
		ParticleModuleSize ModuleSize;
		ParticleModuleNoise ModuleNoise;
		ParticleModuleSubUV ModuleSubUV;

		ParticleContainer Particles;

		BlendMode Blend = BlendMode::Default;
		BillboardMode Billboard = BillboardMode::FaceToCamera;
		TransformationMode Transformation = TransformationMode::World;
		SortMode Sort = SortMode::YoungFirst;

		Vector3 CameraPosition;
	private:
		void SpawnParticles(float TimeTick);
		void UpdateParticles(float TimeTick);
		void RemoveOldParticles();
		void SortParticles();
	public:
		void Update(float TimeTick);

		virtual void Serialize(JSON& J) const final override;
		virtual void Deserialize(JSON& J) final override;
	};

}


