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

	enum E_PARTICLE_SHAPE
	{
		E_PARTICLE_SHAPE_CIRCLE,
		E_PARTICLE_SHAPE_SPHERE,
		E_PARTICLE_SHAPE_CUBE
	};

	enum E_PARTICLE_TRANSFORMATION
	{
		E_PARTICLE_TRANSFORMATION_LOCAL,
		E_PARTICLE_TRANSFORMATION_WORLD
	};

	enum E_PARTICLE_SORT_MODE
	{
		E_PARTICLE_SORT_MODE_NONE,
		E_PARTICLE_SORT_MODE_DISTANCE
	};

	enum E_PARTICLE_SUB_UV_MODE
	{
		E_PARTICLE_SUB_UV_MODE_LINEAR,
		E_PARTICLE_SUB_UV_MODE_RANDOM
	};

	class ParticleEffect
	{
	private:
		Material* mMaterial = nullptr;

		int mParticlesCount = 5;

		bool mVisible = true;
		bool mScaleOverLifetime = false;
		bool mEmitFromShell = false;
		bool mAdditive = false;
		bool mBillboarding = true;
		bool mGradienting = true;

		Vector3 mPos = Vector3(0, 0, 0);
		Vector3 mMinVelocity = Vector3(-1, -1, -1);
		Vector3 mMaxVelocity = Vector3(1, 1, 1);
		Vector3 mMinAcceleration = Vector3(0, 0.1, 0);
		Vector3 mMaxAcceleration = Vector3(0, 0.1, 0);
		Vector3 mConstantForce = Vector3(0, 0, 0);

		Vector2 mParticleSize = Vector2(1, 1);

		Vector2 mStartSize = Vector2(1, 1);
		Vector2 mFinalSize = Vector2(1, 1);
		Vector2 mSubUV = Vector2(1, 1);
		Vector4 mStartColor = Vector4(1, 1, 1, 1);
		Vector4 mFinalColor = Vector4(1, 1, 1, 1);
		Vector3 mBoxShapeSize = Vector3(1, 1, 1);

		float mMinTimeToLive = 1.0;
		float mMaxTimeToLive = 1.0;
		float mMinRotation = 0.0;
		float mMaxRotation = 0.0;
		float mMinRotationSpeed = 0.0;
		float mMaxRotationSpeed = 0.0;
		float mNoiseStrength = 1.0;
		unsigned int mNoiseOctaves = 1;
		float mNoiseLacunarity = 2.0;
		float mNoisePersistence = 0.5;
		float mNoiseFrequency = 1.0;
		float mNoiseAmplitude = 1.0;

		float mEmitRate = 5.0;

		int mParticleTransformation = E_PARTICLE_TRANSFORMATION_WORLD;
		int mParticleShape = E_PARTICLE_SHAPE_CIRCLE;
		float mParticleShapeRadius = 1.0;
		int mSortMode = E_PARTICLE_SORT_MODE_NONE;
		float mSubUVCycles = 1.0;
		int mSubUVMode = E_PARTICLE_SUB_UV_MODE_LINEAR;
	public:
		ParticleEffect();
		ParticleEffect(std::string aFile);
		ParticleEffect(std::string aFile, Material* aMaterial);

		void setMaterial(const Material* aMaterial);
		void setParticlesCount(const int aParticlesCount);
		void setVisible(const bool aVisible);
		void setScaleOverLifetime(const bool aA);
		void setEmitFromShell(const bool aA);
		void setAdditive(const bool aA);
		void setBillboarding(const bool aA);
		void setGradienting(const bool aA);
		void setPos(const Vector3 aPos);
		void addPos(const Vector3 aPos);
		void setMinVelocity(const Vector3 aMinVelocity);
		void setMaxVelocity(const Vector3 aMaxVelocity);
		void setMinAcceleration(const Vector3 aMinAccerleration);
		void setMaxAcceleration(const Vector3 aMaxAcceleration);
		void setConstantForce(const Vector3 aConstantForce);
		void setParticleSize(const Vector2 aParticleSize);
		void setStartSize(const Vector2 aStartSize);
		void setFinalSize(const Vector2 aFinalSize);
		void setSubUV(const Vector2 aSubUV);
		void setStartColor(const Vector4 aStartColor);
		void setFinalColor(const Vector4 aFinalColor);
		void setBoxShapeSize(const Vector3 aBoxShapeSize);
		void setMinTimeToLive(const float aMinTimeToLive);
		void setMaxTimeToLive(const float aMaxTimeToLive);
		void setMinRotation(const float aMinRotation);
		void setMaxRotation(const float aMaxRotation);
		void setMinRotationSpeed(const float aMinRotationSpeed);
		void setMaxRotationSpeed(const float aMaxRotationSpeed);
		void setNoiseStrength(const float aNoiseStrength);
		void setNoiseOctaves(const unsigned int aNoiseOctaves);
		void setNoiseLacunarity(const float aNoiseLacunarity);
		void setNoisePersistence(const float aNoisePersistence);
		void setNoiseFrequency(const float aNoiseFrequency);
		void setNoiseAmplitude(const float aNoiseAmplitude);
		void setEmitRate(const float aEmitRate);
		void setTransformation(const E_PARTICLE_TRANSFORMATION aParticleTransformation);
		void setParticleShape(const E_PARTICLE_SHAPE aParticleShape);
		void setParticleShapeRadius(const float aRadius);
		void setSortMode(const E_PARTICLE_SORT_MODE aSortMode);
		void setSubUVMode(const E_PARTICLE_SUB_UV_MODE aSubUVMode);
		void setSubUVCycles(const float aSubUVCycles);


		Material* getMaterial() const;
		int getParticlesCount() const;
		bool getVisible() const;
		bool getScaleOverLifetime() const;
		bool getEmitFromShell() const;
		bool getAdditive() const;
		bool getBillbiarding() const;
		bool getGradienting() const;
		Vector3 getPos() const;
		Vector3 getMinVelocity() const;
		Vector3 getMaxVelocity() const;
		Vector3 getMinAcceleration() const;
		Vector3 getMaxAcceleration() const;
		Vector3 getConstantForce() const;
		Vector2 getParticleSize() const;
		Vector2 getStartSize() const;
		Vector2 getSubUV() const;
		Vector2 getFinalSize() const;
		Vector4 getStartColor() const;
		Vector4 getFinalColor() const;
		Vector3 getBoxShapeSize() const;
		float getMinTimeToLive() const;
		float getMaxTimeToLive() const;
		float getMinRotation() const;
		float getMaxRotation() const;
		float getMinRotationSpeed() const;
		float getMaxRotationSpeed() const;
		float getNoiseStrength() const;
		unsigned int getNoiseOctaves() const;
		float getNoiseLacunarity() const;
		float getNoisePersistence() const;
		float getNoiseFrequency() const;
		float getNoiseAmplitude() const;
		float getEmitRate() const;
		int getTransformation() const;
		int getParticleShape() const;
		float getParticleShapeRadius() const;
		int getSortMode() const;
		int getSubUVMode() const;
		float getSubUVCycles() const;

		bool saveToXML(std::string aFile) const;
		bool saveToJSON(std::string aFile) const;
		bool loadFromXML(std::string aFile);
		bool loadFromJSON(std::string aFile);
		bool load(std::string aFile);

		~ParticleEffect();
	};

}
