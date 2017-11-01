/************************************************
*              ParticleEffect.h                 *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*             Nikolay(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#pragma once

#include <Graphics/Material.h>
#include <System/Console.h>
#include <System/Serializer.h>

namespace C
{

	enum C_PARTICLE_SHAPE
	{
		C_PARTICLE_SHAPE_CIRCLE,
		C_PARTICLE_SHAPE_SPHERE
	};

	enum C_PARTICLE_TRANSFORMATION
	{
		C_PARTICLE_TRANSFORMATION_LOCAL,
		C_PARTICLE_TRANSFORMATION_WORLD
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
		C_Vector3 mMinDirection = C_Vector3(-1, -1, -1);
		C_Vector3 mMaxDirection = C_Vector3(1, 1, 1);
		C_Vector3 mMinAcceleration = C_Vector3(0, 0.1, 0);
		C_Vector3 mMaxAcceleration = C_Vector3(0, 0.1, 0);
		C_Vector3 mConstantForce = C_Vector3(0, 0, 0);

		C_Vector2 mParticleSize = C_Vector2(1, 1);

		C_Vector2 mStartSize = C_Vector2(1, 1);
		C_Vector2 mFinalSize = C_Vector2(1, 1);
		C_Vector4 mStartColor = C_Vector4(1, 1, 1, 1);
		C_Vector4 mFinalColor = C_Vector4(1, 1, 1, 1);

		float mMinTimeToLive = 1.0;
		float mMaxTimeToLive = 1.0;
		float mMinVelocity = 1.0;
		float mMaxVelocity = 1.0;
		float mMinRotation = 0.0;
		float mMaxRotation = 0.0;
		float mMinRotationSpeed = 0.0;
		float mMaxRotationSpeed = 0.0;

		int mEmitRate = 5;

		int mParticleTransformation = C_PARTICLE_TRANSFORMATION_WORLD;
		int mParticleShape = C_PARTICLE_SHAPE_CIRCLE;
		float mParticleShapeRadius = 1.0;
	public:
		//Constructor 1
		C_ParticleEffect();
		//Constructor 2
		C_ParticleEffect(std::string aFile);

		//Set particles material
		void setMaterial(const C_Material* aMaterial);
		//Set count of particles
		void setParticlesCount(const int aParticlesCount);
		//Set particles visible
		void setVisible(const bool aVisible);
		//Set particles scale over lifetime
		void setScaleOverLifetime(const bool aA);
		//Set particles emit from shell
		void setEmitFromShell(const bool aA);
		//Set particles additive blending
		void setAdditive(const bool aA);
		//Set particles billboarding
		void setBillboarding(const bool aA);
		//Set particles gradianting
		void setGradienting(const bool aA);
		//Set particle emitter position
		void setPos(const C_Vector3 aPos);
		//Add position to current
		void addPos(const C_Vector3 aPos);
		//Set negative direction limit
		void setMinDirection(const C_Vector3 aMinDirection);
		//Set positive direction limit
		void setMaxDirection(const C_Vector3 aMaxDirection);
		//Set particle minimum acceleration
		void setMinAcceleration(const C_Vector3 aMinAccerleration);
		//Set particle maximum acceleration
		void setMaxAcceleration(const C_Vector3 aMaxAcceleration);
		//Set constant force acting on particles
		void setConstantForce(const C_Vector3 aConstantForce);
		//Set particle size
		void setParticleSize(const C_Vector2 aParticleSize);
		//Set particle start size
		void setStartSize(const C_Vector2 aStartSize);
		//Set particle final size
		void setFinalSize(const C_Vector2 aFinalSize);
		//Set particle start color
		void setStartColor(const C_Vector4 aStartColor);
		//Set particle final color
		void setFinalColor(const C_Vector4 aFinalColor);
		//Set particle minimum time to live
		void setMinTimeToLive(const float aMinTimeToLive);
		//Set particle maximum time to live
		void setMaxTimeToLive(const float aMaxTimeToLive);
		//Set particle minimum velocity
		void setMinVelocity(const float aMinVelocity);
		//Set particle maximum velocity
		void setMaxVelocity(const float aMaxVelocity);
		//Set particle minimum rotation
		void setMinRotation(const float aMinRotation);
		//Set particle maximum rotation
		void setMaxRotation(const float aMaxRotation);
		//Set particle minimum rotation speed
		void setMinRotationSpeed(const float aMinRotationSpeed);
		//Set particle maximum rotation speed
		void setMaxRotationSpeed(const float aMaxRotationSpeed);
		//Set particles emit rate
		void setEmitRate(const int aEmitRate);
		//Set particles transformation
		void setTransformation(const C_PARTICLE_TRANSFORMATION aParticleTransformation);
		//Set particle shape
		void setParticleShape(const C_PARTICLE_SHAPE aParticleShape);
		//Set particle shape radius
		void setParticleShapeRadius(const float aRadius);


		//Return material
		C_Material* getMaterial() const;
		//Return particles count
		int getParticlesCount() const;
		//Return particles visible
		bool getVisible() const;
		//Return particles scale over lifetime
		bool getScaleOverLifetime() const;
		//Return particles emit from shell
		bool getEmitFromShell() const;
		//Return particles additive blending
		bool getAdditive() const;
		//Return particles billboarding
		bool getBillbiarding() const;
		//Return particles gradianting
		bool getGradienting() const;
		//Return particle emitter position
		C_Vector3 getPos() const;
		//Return minimum particle direction
		C_Vector3 getMinDirection() const;
		//Return maximum particle direction
		C_Vector3 getMaxDirection() const;
		//Return particle minimum acceleration
		C_Vector3 getMinAcceleration() const;
		//Return particle maximum acceleration
		C_Vector3 getMaxAcceleration() const;
		//Return constant force acting on particles
		C_Vector3 getConstantForce() const;
		//Return particle size
		C_Vector2 getParticleSize() const;
		//Return particle start size
		C_Vector2 getStartSize() const;
		//Return particle final size
		C_Vector2 getFinalSize() const;
		//Return particle start color
		C_Vector4 getStartColor() const;
		//Return particle final color
		C_Vector4 getFinalColor() const;
		//Return particle minimum time to live
		float getMinTimeToLive() const;
		//Return particle maximum time to live
		float getMaxTimeToLive() const;
		//Return particle minimum velocity
		float getMinVelocity() const;
		//Return particle maximum velocity
		float getMaxVelocity() const;
		//Return particle minimum rotation
		float getMinRotation() const;
		//Return partile maximum rotation
		float getMaxRotation() const;
		//Return particle minimum rotation speed
		float getMinRotationSpeed() const;
		//Return particle maximum rotation speed
		float getMaxRotationSpeed() const;
		//Return particles emit rate
		int getEmitRate() const;
		//Return particles transformation
		int getTransformation() const;
		//Return particle shape
		int getParticleShape() const;
		//Return particle shape radius
		float getParticleShapeRadius() const;

		//Serialize to XML file
		bool saveToXML(std::string aFile) const;
		//Deserialize from XML file
		bool loadFromXML(std::string aFile);

		//Destructor
		~C_ParticleEffect();
	};

}
