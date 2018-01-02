/************************************************
*              ParticleEffect.cpp               *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#include <Graphics/ParticleEffect.h>

using nlohmann::json;

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
		mMinRotation(0.0),
		mMaxRotation(0.0),
		mMinRotationSpeed(0.0),
		mEmitRate(5),
		mParticleShape(C_PARTICLE_SHAPE_CIRCLE),
		mParticleShapeRadius(1.0),
		mParticleTransformation(C_PARTICLE_TRANSFORMATION_WORLD),
		mSortMode(C_PARTICLE_SORT_MODE_NONE)
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
		mMinRotation(0.0),
		mMaxRotation(0.0),
		mMinRotationSpeed(0.0),
		mEmitRate(5),
		mParticleShape(C_PARTICLE_SHAPE_CIRCLE),
		mParticleShapeRadius(1.0),
		mParticleTransformation(C_PARTICLE_TRANSFORMATION_WORLD),
		mSortMode(C_PARTICLE_SORT_MODE_NONE)
	{
		load(aFile);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Constructor 3
	C_ParticleEffect::C_ParticleEffect(std::string aFile, C_Material* aMaterial) :
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
		mMinRotation(0.0),
		mMaxRotation(0.0),
		mMinRotationSpeed(0.0),
		mEmitRate(5),
		mParticleShape(C_PARTICLE_SHAPE_CIRCLE),
		mParticleShapeRadius(1.0),
		mParticleTransformation(C_PARTICLE_TRANSFORMATION_WORLD),
		mSortMode(C_PARTICLE_SORT_MODE_NONE)
	{
		load(aFile);
		mMaterial = aMaterial;
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
	//Set particle minimum velocity
	void C_ParticleEffect::setMinVelocity(const C_Vector3 aMinVelocity)
	{
		mMinVelocity = static_cast<C_Vector3>(aMinVelocity);
	}
	//////////////////////////////////////////////////////////////////////////////
	//Set particle maximum velocity
	void C_ParticleEffect::setMaxVelocity(const C_Vector3 aMaxVelocity)
	{
		mMaxVelocity = static_cast<C_Vector3>(aMaxVelocity);
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
	//Set particles mode
	void C_ParticleEffect::setSortMode(const C_PARTICLE_SORT_MODE aSortMode)
	{
		mSortMode = static_cast<int>(aSortMode);
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
	//Return particle minimum velocity
	C_Vector3 C_ParticleEffect::getMinVelocity() const
	{
		return mMinVelocity;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Return particle maximum velocity
	C_Vector3 C_ParticleEffect::getMaxVelocity() const
	{
		return mMaxVelocity;
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
	//Return particles mode
	int C_ParticleEffect::getSortMode() const
	{
		return mSortMode;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Serialize to XML file
	bool C_ParticleEffect::saveToXML(std::string aFile) const
	{
		Serializer::C_SerializerXML serializer;

		if (!serializer.write(aFile, "ParticleEffect"))
		{ C_Log::error("Can't save Particle Effect: " + aFile); return false; }

		if (!serializer.setInt("Count", mParticlesCount))
		{ C_Log::error("Can't save Particles count: " + aFile); return false; }

		if (!serializer.setBool("Visible", mVisible))
		{ C_Log::error("Can't save Particles visible: " + aFile); return false; }

		if (!serializer.setBool("ScaleOL", mScaleOverLifetime))
		{ C_Log::error("Can't save Particles scale over lifetime: " + aFile); return false; }

		if (!serializer.setBool("EmitFromShell", mEmitFromShell))
		{ C_Log::error("Can't save Particles emit from shell: " + aFile); return false; }

		if (!serializer.setBool("AdditiveBlending", mAdditive))
		{ C_Log::error("Can't save Particles additive blending: " + aFile); return false; }

		if (!serializer.setBool("Billboarding", mBillboarding))
		{ C_Log::error("Can't save Particles billboarding: " + aFile); return false; }

		if (!serializer.setBool("Gradienting", mGradienting))
		{ C_Log::error("Can't save Particles gradienting: " + aFile); return false; }

		if (!serializer.setVector3("MinVelocity", mMinVelocity, { "X", "Y", "Z" }))
		{ C_Log::error("Can't save Particles min velocity: " + aFile); return false; }

		if (!serializer.setVector3("MaxVelocity", mMaxVelocity, { "X", "Y", "Z" }))
		{ C_Log::error("Can't save Particles max velocity: " + aFile); return false; }

		if (!serializer.setVector3("MinAcceleration", mMinAcceleration, { "X", "Y", "Z" }))
		{ C_Log::error("Can't save Particles min acceleration: " + aFile); return false; }

		if (!serializer.setVector3("MaxAcceleration", mMaxAcceleration, { "X", "Y", "Z" }))
		{ C_Log::error("Can't save Particles max acceleration: " + aFile); return false; }

		if (!serializer.setVector3("ConstForce", mConstantForce, { "X", "Y", "Z" }))
		{ C_Log::error("Can't save Particles constant force: " + aFile); return false; }

		if (!serializer.setVector2("PartSize", mParticleSize, { "X", "Y" }))
		{ C_Log::error("Can't save Particles size: " + aFile); return false; }

		if (!serializer.setVector2("StartSize", mStartSize, { "X", "Y" }))
		{ C_Log::error("Can't save Particles start size: " + aFile); return false; }

		if (!serializer.setVector2("FinalSize", mFinalSize, { "X", "Y" }))
		{ C_Log::error("Can't save Particles final size: " + aFile); return false; }

		if (!serializer.setVector4("StartColor", mStartColor, { "R", "G", "B", "A" }))
		{ C_Log::error("Can't save Particles start color: ", aFile); return false; }

		if (!serializer.setVector4("FinalColor", mFinalColor, { "R", "G", "B", "A" }))
		{ C_Log::error("Can't save Particles final color: ", aFile); return false; }

		if (!serializer.setFloat("MinTTL", mMinTimeToLive))
		{ C_Log::error("Can't save Particles min TTL: " + aFile); return false; }

		if (!serializer.setFloat("MaxTTL", mMaxTimeToLive))
		{ C_Log::error("Can't save Particles max TTL: " + aFile); return false; }

		if (!serializer.setFloat("MinRotation", mMinRotation))
		{ C_Log::error("Can't save Particles min rotation: " + aFile); return false; }

		if (!serializer.setFloat("MaxRotation", mMaxRotation))
		{ C_Log::error("Can't save Particles max rotation: " + aFile); return false; }

		if (!serializer.setFloat("MinRotationSpeed", mMinRotationSpeed))
		{ C_Log::error("Can't save Particles min rotation speed: " + aFile); return false; }

		if (!serializer.setFloat("MaxRotationSpeed", mMaxRotationSpeed))
		{ C_Log::error("Can't save Particles max rotation speed: " + aFile); return false; }

		if (!serializer.setInt("EmitRate", mEmitRate))
		{ C_Log::error("Can't save Particles emit rate: " + aFile); return false; }

		if (!serializer.setInt("Transformation", mParticleTransformation))
		{ C_Log::error("Can't save Particles transformation: " + aFile); return false; }

		if (!serializer.setInt("Shape", mParticleShape))
		{ C_Log::error("Can't save Particles shape: " + aFile); return false; }

		if (!serializer.setFloat("ShapeRadius", mParticleShapeRadius))
		{ C_Log::error("Can't save Particles shape radius: " + aFile); return false; }

		if (!serializer.setInt("SortMode", mSortMode))
		{ C_Log::error("Can't save Particles sort mode: " + aFile); return false; }

		if (!serializer.save())
		{ C_Log::error("Can't save Particle Effect: " + aFile); return false; }


		C_Log::success("Particle Effect saved: " + aFile);

		return true;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Serialize to JSON file
	bool C_ParticleEffect::saveToJSON(std::string aFile) const
	{
		json j;

		j["ParticleEffect"]["Count"] = mParticlesCount;
		j["ParticleEffect"]["Visible"] = mVisible;
		j["ParticleEffect"]["ScaleOL"] = mScaleOverLifetime;
		j["ParticleEffect"]["EmitFromShell"] = mEmitFromShell;
		j["ParticleEffect"]["AdditiveBlending"] = mAdditive;
		j["ParticleEffect"]["Billboarding"] = mBillboarding;
		j["ParticleEffect"]["Gradienting"] = mGradienting;
		j["ParticleEffect"]["MinVelocity"] = {mMinVelocity.x, mMinVelocity.y, mMinVelocity.z};
		j["ParticleEffect"]["MaxVelocity"] = {mMaxVelocity.x, mMaxVelocity.y, mMaxVelocity.z};
		j["ParticleEffect"]["MinAcceleration"] = {mMinAcceleration.x, mMinAcceleration.y, mMinAcceleration.z};
		j["ParticleEffect"]["MaxAcceleration"] = {mMaxAcceleration.x, mMaxAcceleration.y, mMaxAcceleration.z};
		j["ParticleEffect"]["ConstForce"] = {mConstantForce.x, mConstantForce.y, mConstantForce.z};
		j["ParticleEffect"]["PartSize"] = {mParticleSize.x, mParticleSize.y};
		j["ParticleEffect"]["StartSize"] = {mStartSize.x, mStartSize.y};
		j["ParticleEffect"]["FinalSize"] = {mFinalSize.x, mFinalSize.y};
		j["ParticleEffect"]["StartColor"] = {mStartColor.x, mStartColor.y, mStartColor.z, mStartColor.w};
		j["ParticleEffect"]["FinalColor"] = {mFinalColor.x, mFinalColor.y, mFinalColor.z, mFinalColor.w};
		j["ParticleEffect"]["MinTTL"] = mMinTimeToLive;
		j["ParticleEffect"]["MaxTTL"] = mMaxTimeToLive;
		j["ParticleEffect"]["MinRotation"] = mMinRotation;
		j["ParticleEffect"]["MaxRotation"] = mMaxRotation;
		j["ParticleEffect"]["MinRotationSpeed"] = mMinRotationSpeed;
		j["ParticleEffect"]["MaxRotationSpeed"] = mMaxRotationSpeed;
		j["ParticleEffect"]["EmitRate"] = mEmitRate;
		j["ParticleEffect"]["Transformation"] = mParticleTransformation;
		j["ParticleEffect"]["Shape"] = mParticleShape;
		j["ParticleEffect"]["ShapeRadius"] = mParticleShapeRadius;
		j["ParticleEffect"]["SortMode"] = mSortMode;
	
		std::ofstream o(aFile);
		
		if (o.is_open() == false)
		{
			C_Log::error("Can't save Particle Effect: " + aFile);
			return false;
		}

		o << std::setw(4) << j << std::endl;
		o.close();

		C_Log::success("Particle Effect saved: " + aFile);

		return true;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Serialize from XML file
	bool C_ParticleEffect::loadFromXML(std::string aFile)
	{

		Serializer::C_SerializerXML serializer;

		if (!serializer.read(aFile, "ParticleEffect"))
		{ C_Log::error("Can't load Particle Effect: " + aFile); return false; }

		if (!serializer.getInt("Count", &mParticlesCount))
		{ C_Log::error("Can't load Particles count: " + aFile); return false; }

		if (!serializer.getBool("Visible", &mVisible))
		{ C_Log::error("Can't load Particles visible: " + aFile); return false; }

		if (!serializer.getBool("ScaleOL", &mScaleOverLifetime))
		{ C_Log::error("Can't load Particles scale over lifetime: " + aFile); return false; }

		if (!serializer.getBool("EmitFromShell", &mEmitFromShell))
		{ C_Log::error("Can't load Particles emit from shell: " + aFile); return false; }

		if (!serializer.getBool("AdditiveBlending", &mAdditive))
		{ C_Log::error("Can't load Particles additive blending: " + aFile); return false; }

		if (!serializer.getBool("Billboarding", &mBillboarding))
		{ C_Log::error("Can't load Particles billboarding: " + aFile); return false; }

		if (!serializer.getBool("Gradienting", &mGradienting))
		{ C_Log::error("Can't load Particles gradienting: " + aFile); return false; }

		if (!serializer.getVector3("MinVelocity", &mMinVelocity, { "X", "Y", "Z" }))
		{ C_Log::error("Can't load Particles min velocity: " + aFile); return false; }

		if (!serializer.getVector3("MaxVelocity", &mMaxVelocity, { "X", "Y", "Z" }))
		{ C_Log::error("Can't load Particles max velocity: " + aFile); return false; }

		if (!serializer.getVector3("MinAcceleration", &mMinAcceleration, { "X", "Y", "Z" }))
		{ C_Log::error("Can't load Particles min acceleration: " + aFile); return false; }

		if (!serializer.getVector3("MaxAcceleration", &mMaxAcceleration, { "X", "Y", "Z" }))
		{ C_Log::error("Can't load Particles max acceleration: " + aFile); return false; }

		if (!serializer.getVector3("ConstForce", &mConstantForce, { "X", "Y", "Z" }))
		{ C_Log::error("Can't load Particles constant force: " + aFile); return false; }

		if (!serializer.getVector2("PartSize", &mParticleSize, { "X", "Y"}))
		{ C_Log::error("Can't load Particles size: " + aFile); return false; }

		if (!serializer.getVector2("StartSize", &mStartSize, { "X", "Y" }))
		{ C_Log::error("Can't load Particles start size: " + aFile); return false; }

		if (!serializer.getVector2("FinalSize", &mFinalSize, { "X", "Y" }))
		{ C_Log::error("Can't load Particles final size: " + aFile); return false; }

		if (!serializer.getVector4("StartColor", &mStartColor, { "R", "G", "B", "A"}))
		{ C_Log::error("Can't load Particles start color: " + aFile); return false; }

		if (!serializer.getVector4("FinalColor", &mFinalColor, { "R", "G", "B", "A" }))
		{ C_Log::error("Can't load Particles final color: " + aFile); return false; }

		if (!serializer.getFloat("MinTTL", &mMinTimeToLive))
		{ C_Log::error("Can't load Particles min TTL: " + aFile); return false; }

		if (!serializer.getFloat("MaxTTL", &mMaxTimeToLive))
		{ C_Log::error("Can't load Particles max TTL: " + aFile); return false; }

		if (!serializer.getFloat("MinRotation", &mMinRotation))
		{ C_Log::error("Can't load Particles min rotation: " + aFile); return false; }

		if (!serializer.getFloat("MaxRotation", &mMaxRotation))
		{ C_Log::error("Can't load Particles max rotation: " + aFile); return false; }

		if (!serializer.getFloat("MinRotationSpeed", &mMinRotationSpeed))
		{ C_Log::error("Can't load Particles min rotation speed: " + aFile); return false; }

		if (!serializer.getFloat("MaxRotationSpeed", &mMaxRotationSpeed))
		{ C_Log::error("Can't load Particles max rotation speed: " + aFile); return false; }

		if (!serializer.getInt("EmitRate", &mEmitRate))
		{ C_Log::error("Can't load Particles emit rate: " + aFile); return false; }

		if (!serializer.getInt("Transformation", &mParticleTransformation))
		{ C_Log::error("Can't load Particles transformation: " + aFile); return false; }

		if (!serializer.getInt("Shape", &mParticleShape))
		{ C_Log::error("Can't load Particles shape: " + aFile); return false; }

		if (!serializer.getFloat("ShapeRadius", &mParticleShapeRadius))
		{ C_Log::error("Can't load Particles shape radius: " + aFile); return false; }

		if (!serializer.getInt("SortMode", &mSortMode))
		{ C_Log::error("Can't load Particles sort mode: " + aFile); return false; }

		C_Log::success("Particle Effect loaded: " + aFile);

		return true;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Deserialize from JSON file
	bool C_ParticleEffect::loadFromJSON(std::string aFile)
	{
		std::ifstream i(aFile);
		if (i.is_open() == false)
		{
			C_Log::error("Can't load Particle Effect: " + aFile);
			return false;
		}
		json j;
		i >> j;

		mParticlesCount = j["ParticleEffect"]["Count"];
		mVisible = j["ParticleEffect"]["Visible"];
		mScaleOverLifetime = j["ParticleEffect"]["ScaleOL"];
		mEmitFromShell = j["ParticleEffect"]["EmitFromShell"];
		mAdditive = j["ParticleEffect"]["AdditiveBlending"];
		mBillboarding = j["ParticleEffect"]["Billboarding"];
		mGradienting = j["ParticleEffect"]["Gradienting"];
		mAdditive = j["ParticleEffect"]["AdditiveBlending"];
		mAdditive = j["ParticleEffect"]["AdditiveBlending"];

		mMinVelocity.x = j["ParticleEffect"]["MinVelocity"][0];
		mMinVelocity.y = j["ParticleEffect"]["MinVelocity"][1];
		mMinVelocity.z = j["ParticleEffect"]["MinVelocity"][2];

		mMaxVelocity.x = j["ParticleEffect"]["MaxVelocity"][0];
		mMaxVelocity.y = j["ParticleEffect"]["MaxVelocity"][1];
		mMaxVelocity.z = j["ParticleEffect"]["MaxVelocity"][2];

		mMinAcceleration.x = j["ParticleEffect"]["MinAcceleration"][0];
		mMinAcceleration.y = j["ParticleEffect"]["MinAcceleration"][1];
		mMinAcceleration.z = j["ParticleEffect"]["MinAcceleration"][2];

		mMaxAcceleration.x = j["ParticleEffect"]["MaxAcceleration"][0];
		mMaxAcceleration.y = j["ParticleEffect"]["MaxAcceleration"][1];
		mMaxAcceleration.z = j["ParticleEffect"]["MaxAcceleration"][2];

		mConstantForce.x = j["ParticleEffect"]["ConstForce"][0];
		mConstantForce.y = j["ParticleEffect"]["ConstForce"][1];
		mConstantForce.z = j["ParticleEffect"]["ConstForce"][2];

		mParticleSize.x = j["ParticleEffect"]["PartSize"][0];
		mParticleSize.y = j["ParticleEffect"]["PartSize"][1];

		mStartSize.x = j["ParticleEffect"]["StartSize"][0];
		mStartSize.y = j["ParticleEffect"]["StartSize"][1];

		mFinalSize.x = j["ParticleEffect"]["FinalSize"][0];
		mFinalSize.y = j["ParticleEffect"]["FinalSize"][1];

		mStartColor.x = j["ParticleEffect"]["StartColor"][0];
		mStartColor.y = j["ParticleEffect"]["StartColor"][1];
		mStartColor.z = j["ParticleEffect"]["StartColor"][2];
		mStartColor.w = j["ParticleEffect"]["StartColor"][3];

		mFinalColor.x = j["ParticleEffect"]["FinalColor"][0];
		mFinalColor.y = j["ParticleEffect"]["FinalColor"][1];
		mFinalColor.z = j["ParticleEffect"]["FinalColor"][2];
		mFinalColor.w = j["ParticleEffect"]["FinalColor"][3];

		mMinTimeToLive = j["ParticleEffect"]["MinTTL"];
		mMaxTimeToLive = j["ParticleEffect"]["MaxTTL"];
		mMinRotation = j["ParticleEffect"]["MinRotation"];
		mMaxRotation = j["ParticleEffect"]["MaxRotation"];
		mMinRotationSpeed = j["ParticleEffect"]["MinRotationSpeed"];
		mMaxRotationSpeed = j["ParticleEffect"]["MaxRotationSpeed"];
		mEmitRate = j["ParticleEffect"]["EmitRate"];
		mParticleTransformation = j["ParticleEffect"]["Transformation"];
		mParticleShape = j["ParticleEffect"]["Shape"];
		mParticleShapeRadius = j["ParticleEffect"]["ShapeRadius"];
		mSortMode = j["ParticleEffect"]["SortMode"];

		i.close();

		C_Log::success("Particle Effect loaded: " + aFile);

		return true;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Deserialize from XML or JSON file
	bool C_ParticleEffect::load(std::string aFile)
	{
		if (aFile.find_last_of(".cxpar") != std::string::npos)
			return loadFromXML(aFile);
		else if (aFile.find_last_of(".cjpar") != std::string::npos)
			return loadFromJSON(aFile);
		else return false;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Destructor
	C_ParticleEffect::~C_ParticleEffect() {}

}
