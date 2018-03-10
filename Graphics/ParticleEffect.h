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
#include <System/Log.h>
#include <System/Serializer.h>
#include <json.hpp>

namespace Columbus
{

	enum ParticleShape
	{
		E_PARTICLE_SHAPE_CIRCLE,
		E_PARTICLE_SHAPE_SPHERE,
		E_PARTICLE_SHAPE_BOX
	};

	enum ParticleTransformation
	{
		E_PARTICLE_TRANSFORMATION_LOCAL,
		E_PARTICLE_TRANSFORMATION_WORLD
	};

	enum ParticleSortMode
	{
		E_PARTICLE_SORT_MODE_NONE,
		E_PARTICLE_SORT_MODE_DISTANCE
	};

	enum ParticleSubUVMode
	{
		E_PARTICLE_SUB_UV_MODE_LINEAR,
		E_PARTICLE_SUB_UV_MODE_RANDOM
	};

	struct ParticleRequired
	{
		bool Visible = true;
		bool AdditiveBlending = false;
		bool Billboarding = true;
		ParticleTransformation Transformation = E_PARTICLE_TRANSFORMATION_WORLD;
		ParticleSortMode SortMode = E_PARTICLE_SORT_MODE_DISTANCE;
	};

	struct ParticleEmit
	{
		bool Emitting = true;
		int Count = 5;
		float EmitRate = 5.0f;
	};

	struct ParticleLocation
	{
		bool EmitFromShell = false;
		ParticleShape Shape;
		float Radius;
		Vector3 Size;
	};

	struct ParticleLifetime
	{
		float Min = 1.0f;
		float Max = 1.0f;
	};

	struct ParticleInitialRotation
	{
		bool Active = true;
		float Min = 0.0f;
		float Max = 0.0f;
		float MinVelocity = 0.0f;
		float MaxVelocity = 0.0f;
	};

	struct ParticleInitialVelocity
	{
		Vector3 Min	= Vector3(-1, -1, -1);
		Vector3 Max = Vector3(1, 1, 1);
	};

	struct ParticleVelocityOverLife
	{
		bool Active = true;
	};

	struct ParticleInitialAcceleration
	{
		Vector3 Min = Vector3(0, 0.1, 0);
		Vector3 Max = Vector3(0, 0.1, 0);
	};

	struct ParticleAccelerationOverLife
	{
		bool Active = true;
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

	struct ParticleNoise
	{
		bool Active = false;

		float Strength = 1.0f;
		int Octaves = 1;
		float Lacunarity = 2.0f;
		float Persistence = 0.5f;
		float Frequency = 1.0f;
		float Amplitude = 1.0f;
	};

	struct ParticleSubUV
	{
		ParticleSubUVMode Mode = E_PARTICLE_SUB_UV_MODE_LINEAR;
		int Horizontal = 1;
		int Vertical = 1;
		float Cycles = 1.0f;
	};

	class ParticleEffect
	{
	private:
		Material* mMaterial = nullptr;
		Vector3 mPos = Vector3(0, 0, 0);
	public:
		ParticleRequired Required;
		ParticleEmit Emit;
		ParticleLocation Location;
		ParticleLifetime Lifetime;
		ParticleInitialRotation InitialRotation;
		ParticleInitialVelocity InitialVelocity;
		ParticleVelocityOverLife VelocityOverLife;
		ParticleInitialAcceleration InitialAcceleration;
		ParticleAccelerationOverLife AccerlerationOverLife;
		ParticleInitialSize InitialSize;
		ParticleSizeOverLife SizeOverLife;
		ParticleInitialColor InitialColor;
		ParticleColorOverLife ColorOverLife;
		ParticleNoise Noise;
		ParticleSubUV SubUV;
	public:
		ParticleEffect();
		ParticleEffect(std::string aFile);
		ParticleEffect(std::string aFile, Material* aMaterial);

		void setMaterial(const Material* aMaterial);
		void setPos(const Vector3 aPos);
		void addPos(const Vector3 aPos);

		Material* getMaterial() const;
		Vector3 getPos() const;

		bool saveToXML(std::string aFile) const;
		bool saveToJSON(std::string aFile) const;
		bool loadFromXML(std::string aFile);
		bool loadFromJSON(std::string aFile);
		bool load(std::string aFile);

		~ParticleEffect();
	};

}
