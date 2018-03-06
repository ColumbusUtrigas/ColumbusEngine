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

	class C_ParticleEffect
	{
	private:
		C_Material* mMaterial = nullptr;

		int mParticlesCount = 5;

		bool mVisible = true;
		bool mScaleOverLifetime = false;
		bool mEmitFromShell = false;
		bool mAdditive = false;
		bool mBillboarding = true;
		bool mGradienting = true;

		C_Vector3 mPos = C_Vector3(0, 0, 0);
		C_Vector3 mMinVelocity = C_Vector3(-1, -1, -1);
		C_Vector3 mMaxVelocity = C_Vector3(1, 1, 1);
		C_Vector3 mMinAcceleration = C_Vector3(0, 0.1, 0);
		C_Vector3 mMaxAcceleration = C_Vector3(0, 0.1, 0);
		C_Vector3 mConstantForce = C_Vector3(0, 0, 0);

		C_Vector2 mParticleSize = C_Vector2(1, 1);

		C_Vector2 mStartSize = C_Vector2(1, 1);
		C_Vector2 mFinalSize = C_Vector2(1, 1);
		C_Vector2 mSubUV = C_Vector2(1, 1);
		C_Vector4 mStartColor = C_Vector4(1, 1, 1, 1);
		C_Vector4 mFinalColor = C_Vector4(1, 1, 1, 1);
		C_Vector3 mBoxShapeSize = C_Vector3(1, 1, 1);

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
		C_ParticleEffect();
		C_ParticleEffect(std::string aFile);
		C_ParticleEffect(std::string aFile, C_Material* aMaterial);

		void setMaterial(const C_Material* aMaterial);
		void setParticlesCount(const int aParticlesCount);
		void setVisible(const bool aVisible);
		void setScaleOverLifetime(const bool aA);
		void setEmitFromShell(const bool aA);
		void setAdditive(const bool aA);
		void setBillboarding(const bool aA);
		void setGradienting(const bool aA);
		void setPos(const C_Vector3 aPos);
		void addPos(const C_Vector3 aPos);
		void setMinVelocity(const C_Vector3 aMinVelocity);
		void setMaxVelocity(const C_Vector3 aMaxVelocity);
		void setMinAcceleration(const C_Vector3 aMinAccerleration);
		void setMaxAcceleration(const C_Vector3 aMaxAcceleration);
		void setConstantForce(const C_Vector3 aConstantForce);
		void setParticleSize(const C_Vector2 aParticleSize);
		void setStartSize(const C_Vector2 aStartSize);
		void setFinalSize(const C_Vector2 aFinalSize);
		void setSubUV(const C_Vector2 aSubUV);
		void setStartColor(const C_Vector4 aStartColor);
		void setFinalColor(const C_Vector4 aFinalColor);
		void setBoxShapeSize(const C_Vector3 aBoxShapeSize);
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


		C_Material* getMaterial() const;
		int getParticlesCount() const;
		bool getVisible() const;
		bool getScaleOverLifetime() const;
		bool getEmitFromShell() const;
		bool getAdditive() const;
		bool getBillbiarding() const;
		bool getGradienting() const;
		C_Vector3 getPos() const;
		C_Vector3 getMinVelocity() const;
		C_Vector3 getMaxVelocity() const;
		C_Vector3 getMinAcceleration() const;
		C_Vector3 getMaxAcceleration() const;
		C_Vector3 getConstantForce() const;
		C_Vector2 getParticleSize() const;
		C_Vector2 getStartSize() const;
		C_Vector2 getSubUV() const;
		C_Vector2 getFinalSize() const;
		C_Vector4 getStartColor() const;
		C_Vector4 getFinalColor() const;
		C_Vector3 getBoxShapeSize() const;
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

		~C_ParticleEffect();
	};

}
