/************************************************
*              ParticleEffect.cpp               *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*             Nikolay(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#include <Graphics/ParticleEffect.h>

namespace Columbus
{
	//////////////////////////////////////////////////////////////////////////////
	//Constructor 1
	C_ParticleEffect::C_ParticleEffect() :
		mMaterial(nullptr),
		mParticlesCount(5),
		mVisible(true),
		mScaleOverLifetime(false),
		mEmitFromShell(false),
		mAdditive(false),
		mBillboarding(true),
		mGradienting(true),
		mMinTimeToLive(1.0),
		mMaxTimeToLive(1.0),
		mMinVelocity(1.0),
		mMaxVelocity(1.0),
		mMinRotation(0.0),
		mMaxRotation(0.0),
		mMinRotationSpeed(0.0),
		mEmitRate(5),
		mParticleShape(C_PARTICLE_SHAPE_CIRCLE),
		mParticleShapeRadius(1.0),
		mParticleTransformation(C_PARTICLE_TRANSFORMATION_WORLD)
	{

	}
	//////////////////////////////////////////////////////////////////////////////
	//Constructor 2
	C_ParticleEffect::C_ParticleEffect(std::string aFile) :
		mMaterial(nullptr),
		mParticlesCount(5),
		mVisible(true),
		mScaleOverLifetime(false),
		mEmitFromShell(false),
		mAdditive(false),
		mBillboarding(true),
		mGradienting(true),
		mMinTimeToLive(1.0),
		mMaxTimeToLive(1.0),
		mMinVelocity(1.0),
		mMaxVelocity(1.0),
		mMinRotation(0.0),
		mMaxRotation(0.0),
		mMinRotationSpeed(0.0),
		mEmitRate(5),
		mParticleShape(C_PARTICLE_SHAPE_CIRCLE),
		mParticleShapeRadius(1.0),
		mParticleTransformation(C_PARTICLE_TRANSFORMATION_WORLD)
	{
		loadFromXML(aFile);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//Set particle material
	void C_ParticleEffect::setMaterial(const C_Material* aMaterial)
	{
		mMaterial = const_cast<C_Material*>(aMaterial);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set count of particles
	void C_ParticleEffect::setParticlesCount(const int aParticlesCount)
	{
		mParticlesCount = static_cast<int>(aParticlesCount);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set particles visible
	void C_ParticleEffect::setVisible(const bool aVisible)
	{
		mVisible = static_cast<bool>(aVisible);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set particles scale over lifetime
	void C_ParticleEffect::setScaleOverLifetime(const bool aA)
	{
		mScaleOverLifetime = static_cast<bool>(aA);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set particles emit from shell
	void C_ParticleEffect::setEmitFromShell(const bool aA)
	{
		mEmitFromShell = static_cast<bool>(aA);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set particles additive blending
	void C_ParticleEffect::setAdditive(const bool aA)
	{
		mAdditive = static_cast<bool>(aA);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set particles billboarding
	void C_ParticleEffect::setBillboarding(const bool aA)
	{
		mBillboarding = static_cast<bool>(aA);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set particles gradianting
	void C_ParticleEffect::setGradienting(const bool aA)
	{
		mGradienting = static_cast<bool>(aA);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set particle emitter position
	void C_ParticleEffect::setPos(const C_Vector3 aPos)
	{
		mPos = static_cast<C_Vector3>(aPos);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Add position to current
	void C_ParticleEffect::addPos(const C_Vector3 aPos)
	{
		mPos += static_cast<C_Vector3>(aPos);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set negative direction limit
	void C_ParticleEffect::setMinDirection(const C_Vector3 aMinDirection)
	{
		mMinDirection = static_cast<C_Vector3>(aMinDirection);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set positive direction limit
	void C_ParticleEffect::setMaxDirection(const C_Vector3 aMaxDirection)
	{
		mMaxDirection = static_cast<C_Vector3>(aMaxDirection);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set particle minimum acceleration
	void C_ParticleEffect::setMinAcceleration(const C_Vector3 aMinAcceleration)
	{
		mMinAcceleration = static_cast<C_Vector3>(aMinAcceleration);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set particle maximum acceleration
	void C_ParticleEffect::setMaxAcceleration(const C_Vector3 aMaxAcceleration)
	{
		mMaxAcceleration = static_cast<C_Vector3>(aMaxAcceleration);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set constant force acting on particles
	void C_ParticleEffect::setConstantForce(const C_Vector3 aConstantForce)
	{
		mConstantForce = static_cast<C_Vector3>(aConstantForce);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set particle size
	void C_ParticleEffect::setParticleSize(const C_Vector2 aParticleSize)
	{
		mParticleSize = static_cast<C_Vector2>(aParticleSize);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set particle start size
	void C_ParticleEffect::setStartSize(C_Vector2 aStartSize)
	{
		mStartSize = static_cast<C_Vector2>(aStartSize);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set particle final size
	void C_ParticleEffect::setFinalSize(C_Vector2 aFinalSize)
	{
		mFinalSize = static_cast<C_Vector2>(aFinalSize);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set particle start color
	void C_ParticleEffect::setStartColor(C_Vector4 aStartColor)
	{
		mStartColor = static_cast<C_Vector4>(aStartColor);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set particle final color
	void C_ParticleEffect::setFinalColor(C_Vector4 aFinalColor)
	{
		mFinalColor = static_cast<C_Vector4>(aFinalColor);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set paritcle minimum time to live
	void C_ParticleEffect::setMinTimeToLive(const float aMinTimeToLive)
	{
		mMinTimeToLive = static_cast<float>(aMinTimeToLive);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set particle maximum time to live
	void C_ParticleEffect::setMaxTimeToLive(const float aMaxTimeToLive)
	{
		mMaxTimeToLive = static_cast<float>(aMaxTimeToLive);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set particle minimum velocity
	void C_ParticleEffect::setMinVelocity(const float aMinVelocity)
	{
		mMinVelocity = static_cast<float>(aMinVelocity);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set particle maximum velocity
	void C_ParticleEffect::setMaxVelocity(const float aMaxVelocity)
	{
		mMaxVelocity = static_cast<float>(aMaxVelocity);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set particle minimum roation
	void C_ParticleEffect::setMinRotation(const float aMinRotation)
	{
		mMinRotation = static_cast<float>(aMinRotation);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set particle maximum rotation
	void C_ParticleEffect::setMaxRotation(const float aMaxRotation)
	{
		mMaxRotation = static_cast<float>(aMaxRotation);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set particle minimum rotation speed
	void C_ParticleEffect::setMinRotationSpeed(const float aMinRotationSpeed)
	{
		mMinRotationSpeed = static_cast<float>(aMinRotationSpeed);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set particle maximum rotation speed
	void C_ParticleEffect::setMaxRotationSpeed(const float aMaxRotationSpeed)
	{
		mMaxRotationSpeed = static_cast<float>(aMaxRotationSpeed);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set particles emit rate
	void C_ParticleEffect::setEmitRate(const int aEmitRate)
	{
		mEmitRate = static_cast<int>(aEmitRate);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set particles transformation
	void C_ParticleEffect::setTransformation(const C_PARTICLE_TRANSFORMATION aParticleTransformation)
	{
		mParticleTransformation = static_cast<int>(aParticleTransformation);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set particle shape
	void C_ParticleEffect::setParticleShape(const C_PARTICLE_SHAPE aParticleShape)
	{
		mParticleShape = static_cast<int>(aParticleShape);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEffect::setParticleShapeRadius(const float aRadius)
	{
		mParticleShapeRadius = static_cast<float>(aRadius);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//Return material
	C_Material* C_ParticleEffect::getMaterial() const
	{
		return mMaterial;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return particles count
	int C_ParticleEffect::getParticlesCount() const
	{
		return mParticlesCount;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return particles visible
	bool C_ParticleEffect::getVisible() const
	{
		return mVisible;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return particles scale over lifetime
	bool C_ParticleEffect::getScaleOverLifetime() const
	{
		return mScaleOverLifetime;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return particles emit from shell
	bool C_ParticleEffect::getEmitFromShell() const
	{
		return mEmitFromShell;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return particles additive blending
	bool C_ParticleEffect::getAdditive() const
	{
		return mAdditive;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return particles billboarding
	bool C_ParticleEffect::getBillbiarding() const
	{
		return mBillboarding;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return particles gradianting
	bool C_ParticleEffect::getGradienting() const
	{
		return mGradienting;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return particle emitter position
	C_Vector3 C_ParticleEffect::getPos() const
	{
		return mPos;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return minimum particle direction
	C_Vector3 C_ParticleEffect::getMinDirection() const
	{
		return mMinDirection;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return maximum particle direction
	C_Vector3 C_ParticleEffect::getMaxDirection() const
	{
		return mMaxDirection;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return particle minimum acceleration
	C_Vector3 C_ParticleEffect::getMinAcceleration() const
	{
		return mMinAcceleration;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return particle maximum acceleration
	C_Vector3 C_ParticleEffect::getMaxAcceleration() const
	{
		return mMaxAcceleration;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return constant force acting on particles
	C_Vector3 C_ParticleEffect::getConstantForce() const
	{
		return mConstantForce;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return particle size
	C_Vector2 C_ParticleEffect::getParticleSize() const
	{
		return mParticleSize;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return particle start size
	C_Vector2 C_ParticleEffect::getStartSize() const
	{
		return mStartSize;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return particle final size
	C_Vector2 C_ParticleEffect::getFinalSize() const
	{
		return mFinalSize;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return particle start color
	C_Vector4 C_ParticleEffect::getStartColor() const
	{
		return mStartColor;;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return particle final color
	C_Vector4 C_ParticleEffect::getFinalColor() const
	{
		return mFinalColor;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return particle minimum time to live
	float C_ParticleEffect::getMinTimeToLive() const
	{
		return mMinTimeToLive;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return particle maximum time to live
	float C_ParticleEffect::getMaxTimeToLive() const
	{
		return mMaxTimeToLive;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return particle minimum velocity
	float C_ParticleEffect::getMinVelocity() const
	{
		return mMinVelocity;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return particle maximum velocity
	float C_ParticleEffect::getMaxVelocity() const
	{
		return mMaxVelocity;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return particle minimum rotation
	float C_ParticleEffect::getMinRotation() const
	{
		return mMinRotation;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return particle maximum rotation
	float C_ParticleEffect::getMaxRotation() const
	{
		return mMaxRotation;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return particle minimum rotation speed
	float C_ParticleEffect::getMinRotationSpeed() const
	{
		return mMinRotationSpeed;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return particle maximum rotation speed
	float C_ParticleEffect::getMaxRotationSpeed() const
	{
		return mMaxRotationSpeed;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return particles emit rate
	int C_ParticleEffect::getEmitRate() const
	{
		return mEmitRate;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return particles transformation
	int C_ParticleEffect::getTransformation() const
	{
		return mParticleTransformation;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Retun particle shape
	int C_ParticleEffect::getParticleShape() const
	{
		return mParticleShape;
	}
	//Return particle shape radius
	float C_ParticleEffect::getParticleShapeRadius() const
	{
		return mParticleShapeRadius;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Serialize to XML file
	bool C_ParticleEffect::saveToXML(std::string aFile) const
	{
		Serializer::C_SerializerXML serializer;

		if (!serializer.write(aFile, "ParticleEffect"))
		{ C_Error("Can't save Particle Effect: %s", aFile); return false; }

		if (!serializer.setInt("Count", mParticlesCount))
		{ C_Error("Can't save Particles count: %s", aFile); return false; }

		if (!serializer.setBool("Visible", mVisible))
		{ C_Error("Can't save Particles visible: %s", aFile); return false; }

		if (!serializer.setBool("ScaleOL", mScaleOverLifetime))
		{ C_Error("Can't save Particles scale over lifetime: %s", aFile); return false; }

		if (!serializer.setBool("EmitFromShell", mEmitFromShell))
		{ C_Error("Can't save Particles emit from shell: %s", aFile); return false; }

		if (!serializer.setBool("AdditiveBlending", mAdditive))
		{ C_Error("Can't save Particles additive blending: %s", aFile); return false; }

		if (!serializer.setBool("Billboarding", mBillboarding))
		{ C_Error("Can't save Particles billboarding: %s", aFile); return false; }

		if (!serializer.setBool("Gradienting", mGradienting))
		{ C_Error("Can't save Particles gradienting: %s", aFile); return false; }

		if (!serializer.setVector3("MinDirection", mMinDirection, { "X", "Y", "Z" }))
		{ C_Error("Can't save Particles min direction: %s", aFile); return false; }

		if (!serializer.setVector3("MaxDirection", mMaxDirection, { "X", "Y", "Z" }))
		{ C_Error("Can't save Particles max direction: %s", aFile); return false; }

		if (!serializer.setVector3("MinAcceleration", mMinAcceleration, { "X", "Y", "Z" }))
		{ C_Error("Can't save Particles min acceleration: %s", aFile); return false; }

		if (!serializer.setVector3("MaxAcceleration", mMaxAcceleration, { "X", "Y", "Z" }))
		{ C_Error("Can't save Particles max acceleration: %s", aFile); return false; }

		if (!serializer.setVector3("ConstForce", mConstantForce, { "X", "Y", "Z" }))
		{ C_Error("Can't save Particles constant force: %s", aFile); return false; }

		if (!serializer.setVector2("PartSize", mParticleSize, { "X", "Y" }))
		{ C_Error("Can't save Particles size: %s", aFile); return false; }

		if (!serializer.setVector2("StartSize", mStartSize, { "X", "Y" }))
		{ C_Error("Can't save Particles start size: %s", aFile); return false; }

		if (!serializer.setVector2("FinalSize", mFinalSize, { "X", "Y" }))
		{ C_Error("Can't save Particles final size: %s", aFile); return false; }

		if (!serializer.setVector4("StartColor", mStartColor, { "R", "G" "B", "A" }))
		{ C_Error("Can't save Particles start color: %s", aFile); return false; }

		if (!serializer.setVector4("FinalColor", mFinalColor, { "R", "G" "B", "A" }))
		{ C_Error("Can't save Particles final color: %s", aFile); return false; }

		if (!serializer.setFloat("MinTTL", mMinTimeToLive))
		{ C_Error("Can't save Particles min TTL: %s", aFile); return false; }

		if (!serializer.setFloat("MaxTTL", mMaxTimeToLive))
		{ C_Error("Can't save Particles max TTL: %s", aFile); return false; }

		if (!serializer.setFloat("MinVelocity", mMinVelocity))
		{ C_Error("Can't save Particles min velocity: %s", aFile); return false; }

		if (!serializer.setFloat("MaxVelocity", mMaxVelocity))
		{ C_Error("Can't save Particles max velocity: %s", aFile); return false; }

		if (!serializer.setFloat("MinRotation", mMinRotation))
		{ C_Error("Can't save Particles min rotation: %s", aFile); return false; }

		if (!serializer.setFloat("MaxRotation", mMaxRotation))
		{ C_Error("Can't save Particles max rotation: %s", aFile); return false; }

		if (!serializer.setFloat("MinRotationSpeed", mMinRotationSpeed))
		{ C_Error("Can't save Particles min rotation speed: %s", aFile); return false; }

		if (!serializer.setFloat("MaxRotationSpeed", mMaxRotationSpeed))
		{ C_Error("Can't save Particles max rotation speed: %s", aFile); return false; }

		if (!serializer.setInt("EmitRate", mEmitRate))
		{ C_Error("Can't save Particles emit rate: %s", aFile); return false; }

		if (!serializer.setInt("Transformation", mParticleTransformation))
		{ C_Error("Can't save Particles transformation: %s", aFile); return false; }

		if (!serializer.setInt("Shape", mParticleShape))
		{ C_Error("Can't save Particles shape: %s", aFile); return false; }

		if (!serializer.setInt("ShapeRadius", mParticleShapeRadius))
		{ C_Error("Can't save Particles shape radius: %s", aFile); return false; }

		if (!serializer.save())
		{ C_Error("Can't save Particle Effect: %s", aFile); return false; }


		C_Success("Particle Effect saved: %s", aFile);

		return true;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Serialize from XML file
	bool C_ParticleEffect::loadFromXML(std::string aFile)
	{

		Serializer::C_SerializerXML serializer;

		if (!serializer.read(aFile, "ParticleEffect"))
		{ C_Error("Can't load Particle Effect: %s", aFile); return false; }

		if (!serializer.getInt("Count", &mParticlesCount))
		{ C_Error("Can't load Particles count: %s", aFile); return false; }

		if (!serializer.getBool("Visible", &mVisible))
		{ C_Error("Can't load Particles visible: %s", aFile); return false; }

		if (!serializer.getBool("ScaleOL", &mScaleOverLifetime))
		{ C_Error("Can't load Particles scale over lifetime: %s", aFile); return false; }

		if (!serializer.getBool("EmitFromShell", &mEmitFromShell))
		{ C_Error("Can't load Particles emit from shell: %s", aFile); return false; }

		if (!serializer.getBool("AdditiveBlending", &mAdditive))
		{ C_Error("Can't load Particles additive blending: %s", aFile); return false; }

		if (!serializer.getBool("Billboarding", &mBillboarding))
		{ C_Error("Can't load Particles billboarding: %s", aFile); return false; }

		if (!serializer.getBool("Gradienting", &mGradienting))
		{ C_Error("Can't load Particles gradienting: %s", aFile); return false; }

		if (!serializer.getVector3("MinDirection", &mMinDirection, { "X", "Y", "Z" }))
		{ C_Error("Can't load Particles min direction: %s", aFile); return false; }

		if (!serializer.getVector3("MaxDirection", &mMaxDirection, { "X", "Y", "Z" }))
		{ C_Error("Can't load Particles max direction: %s", aFile); return false; }

		if (!serializer.getVector3("MinAcceleration", &mMinAcceleration, { "X", "Y", "Z" }))
		{ C_Error("Can't load Particles min acceleration: %s", aFile); return false; }

		if (!serializer.getVector3("MaxAcceleration", &mMaxAcceleration, { "X", "Y", "Z" }))
		{ C_Error("Can't load Particles max acceleration: %s", aFile); return false; }

		if (!serializer.getVector3("ConstForce", &mConstantForce, { "X", "Y", "Z" }))
		{ C_Error("Can't load Particles constant force: %s", aFile); return false; }

		if (!serializer.getVector2("PartSize", &mParticleSize, { "X", "Y"}))
		{ C_Error("Can't load Particles size: %s", aFile); return false; }

		if (!serializer.getVector2("StartSize", &mStartSize, { "X", "Y" }))
		{ C_Error("Can't load Particles start size: %s", aFile); return false; }

		if (!serializer.getVector2("FinalSize", &mFinalSize, { "X", "Y" }))
		{ C_Error("Can't load Particles final size: %s", aFile); return false; }

		if (!serializer.getVector4("StartColor", &mStartColor, { "R", "G", "B", "A"}))
		{ C_Error("Can't load Particles start color: %s", aFile); return false; }

		if (!serializer.getVector4("FinalColor", &mFinalColor, { "R", "G", "B", "A" }))
		{ C_Error("Can't load Particles final color: %s", aFile); return false; }

		if (!serializer.getFloat("MinTTL", &mMinTimeToLive))
		{ C_Error("Can't load Particles min TTL: %s", aFile); return false; }

		if (!serializer.getFloat("MaxTTL", &mMaxTimeToLive))
		{ C_Error("Can't load Particles max TTL: %s", aFile); return false; }

		if (!serializer.getFloat("MinVelocity", &mMinVelocity))
		{ C_Error("Can't load Particles min velocity: %s", aFile); return false; }

		if (!serializer.getFloat("MaxVelocity", &mMaxVelocity))
		{ C_Error("Can't load Particles max velocity: %s", aFile); return false; }

		if (!serializer.getFloat("MinRotation", &mMinRotation))
		{ C_Error("Can't load Particles min rotation: %s", aFile); return false; }

		if (!serializer.getFloat("MaxRotation", &mMaxRotation))
		{ C_Error("Can't load Particles max rotation: %s", aFile); return false; }

		if (!serializer.getFloat("MinRotationSpeed", &mMinRotationSpeed))
		{ C_Error("Can't load Particles min rotation speed: %s", aFile); return false; }

		if (!serializer.getFloat("MaxRotationSpeed", &mMaxRotationSpeed))
		{ C_Error("Can't load Particles max rotation speed: %s", aFile); return false; }

		if (!serializer.getInt("EmitRate", &mEmitRate))
		{ C_Error("Can't load Particles emit rate: %s", aFile); return false; }

		if (!serializer.getInt("Transformation", &mParticleTransformation))
		{ C_Error("Can't load Particles transformation: %s", aFile); return false; }

		if (!serializer.getInt("Shape", &mParticleShape))
		{ C_Error("Can't load Particles shape: %s", aFile); return false; }

		if (!serializer.getFloat("ShapeRadius", &mParticleShapeRadius))
		{ C_Error("Can't load Particles shape radius: %s", aFile); return false; }

		C_Success("Particle Effect loaded: %s", aFile);

		return true;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Destructor
	C_ParticleEffect::~C_ParticleEffect() {}

}
