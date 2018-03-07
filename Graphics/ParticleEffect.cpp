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
		mMaxRotationSpeed(0.0),
		mNoiseStrength(1.0),
		mNoiseOctaves(1),
		mNoiseLacunarity(2.0),
		mNoisePersistence(0.5),
		mNoiseFrequency(1.0),
		mNoiseAmplitude(1.0),
		mEmitRate(5.0),
		mParticleShape(E_PARTICLE_SHAPE_CIRCLE),
		mParticleShapeRadius(1.0),
		mParticleTransformation(E_PARTICLE_TRANSFORMATION_WORLD),
		mSortMode(E_PARTICLE_SORT_MODE_NONE)
	{

	}
	//////////////////////////////////////////////////////////////////////////////
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
		mMaxRotationSpeed(0.0),
		mNoiseStrength(1.0),
		mNoiseOctaves(1),
		mNoiseLacunarity(2.0),
		mNoisePersistence(0.5),
		mNoiseFrequency(1.0),
		mNoiseAmplitude(1.0),
		mEmitRate(5.0),
		mParticleShape(E_PARTICLE_SHAPE_CIRCLE),
		mParticleShapeRadius(1.0),
		mParticleTransformation(E_PARTICLE_TRANSFORMATION_WORLD),
		mSortMode(E_PARTICLE_SORT_MODE_NONE)
	{
		load(aFile);
	}
	//////////////////////////////////////////////////////////////////////////////
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
		mMaxRotationSpeed(0.0),
		mNoiseStrength(1.0),
		mNoiseOctaves(1),
		mNoiseLacunarity(2.0),
		mNoisePersistence(0.5),
		mNoiseFrequency(1.0),
		mNoiseAmplitude(1.0),
		mEmitRate(5.0),
		mParticleShape(E_PARTICLE_SHAPE_CIRCLE),
		mParticleShapeRadius(1.0),
		mParticleTransformation(E_PARTICLE_TRANSFORMATION_WORLD),
		mSortMode(E_PARTICLE_SORT_MODE_NONE)
	{
		load(aFile);
		mMaterial = aMaterial;
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEffect::setMaterial(const C_Material* aMaterial)
	{
		mMaterial = const_cast<C_Material*>(aMaterial);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEffect::setParticlesCount(const int aParticlesCount)
	{
		mParticlesCount = static_cast<int>(aParticlesCount);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEffect::setVisible(const bool aVisible)
	{
		mVisible = static_cast<bool>(aVisible);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEffect::setScaleOverLifetime(const bool aA)
	{
		mScaleOverLifetime = static_cast<bool>(aA);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEffect::setEmitFromShell(const bool aA)
	{
		mEmitFromShell = static_cast<bool>(aA);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEffect::setAdditive(const bool aA)
	{
		mAdditive = static_cast<bool>(aA);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEffect::setBillboarding(const bool aA)
	{
		mBillboarding = static_cast<bool>(aA);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEffect::setGradienting(const bool aA)
	{
		mGradienting = static_cast<bool>(aA);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEffect::setPos(const C_Vector3 aPos)
	{
		mPos = static_cast<C_Vector3>(aPos);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEffect::addPos(const C_Vector3 aPos)
	{
		mPos += static_cast<C_Vector3>(aPos);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEffect::setMinVelocity(const C_Vector3 aMinVelocity)
	{
		mMinVelocity = static_cast<C_Vector3>(aMinVelocity);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEffect::setMaxVelocity(const C_Vector3 aMaxVelocity)
	{
		mMaxVelocity = static_cast<C_Vector3>(aMaxVelocity);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEffect::setMinAcceleration(const C_Vector3 aMinAcceleration)
	{
		mMinAcceleration = static_cast<C_Vector3>(aMinAcceleration);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEffect::setMaxAcceleration(const C_Vector3 aMaxAcceleration)
	{
		mMaxAcceleration = static_cast<C_Vector3>(aMaxAcceleration);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEffect::setConstantForce(const C_Vector3 aConstantForce)
	{
		mConstantForce = static_cast<C_Vector3>(aConstantForce);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEffect::setParticleSize(const C_Vector2 aParticleSize)
	{
		mParticleSize = static_cast<C_Vector2>(aParticleSize);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEffect::setStartSize(const C_Vector2 aStartSize)
	{
		mStartSize = static_cast<C_Vector2>(aStartSize);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEffect::setFinalSize(const C_Vector2 aFinalSize)
	{
		mFinalSize = static_cast<C_Vector2>(aFinalSize);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEffect::setSubUV(const C_Vector2 aSubUV)
	{
		mSubUV = static_cast<C_Vector2>(aSubUV);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEffect::setStartColor(const C_Vector4 aStartColor)
	{
		mStartColor = static_cast<C_Vector4>(aStartColor);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEffect::setFinalColor(const C_Vector4 aFinalColor)
	{
		mFinalColor = static_cast<C_Vector4>(aFinalColor);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEffect::setBoxShapeSize(const C_Vector3 aBoxShapeSize)
	{
		mBoxShapeSize = static_cast<C_Vector3>(aBoxShapeSize);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEffect::setMinTimeToLive(const float aMinTimeToLive)
	{
		mMinTimeToLive = static_cast<float>(aMinTimeToLive);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEffect::setMaxTimeToLive(const float aMaxTimeToLive)
	{
		mMaxTimeToLive = static_cast<float>(aMaxTimeToLive);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEffect::setMinRotation(const float aMinRotation)
	{
		mMinRotation = static_cast<float>(aMinRotation);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEffect::setMaxRotation(const float aMaxRotation)
	{
		mMaxRotation = static_cast<float>(aMaxRotation);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEffect::setMinRotationSpeed(const float aMinRotationSpeed)
	{
		mMinRotationSpeed = static_cast<float>(aMinRotationSpeed);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEffect::setMaxRotationSpeed(const float aMaxRotationSpeed)
	{
		mMaxRotationSpeed = static_cast<float>(aMaxRotationSpeed);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEffect::setNoiseStrength(const float aNoiseStrength)
	{
		mNoiseStrength = static_cast<float>(aNoiseStrength);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEffect::setNoiseOctaves(const unsigned int aNoiseOctaves)
	{
		mNoiseOctaves = static_cast<unsigned int>(aNoiseOctaves);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEffect::setNoiseLacunarity(const float aNoiseLacunarity)
	{
		mNoiseLacunarity = static_cast<float>(aNoiseLacunarity);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEffect::setNoisePersistence(const float aNoisePersistence)
	{
		mNoisePersistence = static_cast<float>(aNoisePersistence);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEffect::setNoiseFrequency(const float aNoiseFrequency)
	{
		mNoiseFrequency = static_cast<float>(aNoiseFrequency);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEffect::setNoiseAmplitude(const float aNoiseAmplitude)
	{
		mNoiseAmplitude = static_cast<float>(aNoiseAmplitude);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEffect::setEmitRate(const float aEmitRate)
	{
		mEmitRate = static_cast<float>(aEmitRate);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEffect::setTransformation(const E_PARTICLE_TRANSFORMATION aParticleTransformation)
	{
		mParticleTransformation = static_cast<int>(aParticleTransformation);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEffect::setParticleShape(const E_PARTICLE_SHAPE aParticleShape)
	{
		mParticleShape = static_cast<int>(aParticleShape);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEffect::setParticleShapeRadius(const float aRadius)
	{
		mParticleShapeRadius = static_cast<float>(aRadius);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEffect::setSortMode(const E_PARTICLE_SORT_MODE aSortMode)
	{
		mSortMode = static_cast<int>(aSortMode);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEffect::setSubUVMode(const E_PARTICLE_SUB_UV_MODE aSubUVMode)
	{
		mSubUVMode = static_cast<int>(aSubUVMode);
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_ParticleEffect::setSubUVCycles(const float aSubUVCycles)
	{
		mSubUVCycles = static_cast<float>(aSubUVCycles);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	C_Material* C_ParticleEffect::getMaterial() const
	{
		return mMaterial;
	}
	//////////////////////////////////////////////////////////////////////////////
	int C_ParticleEffect::getParticlesCount() const
	{
		return mParticlesCount;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool C_ParticleEffect::getVisible() const
	{
		return mVisible;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool C_ParticleEffect::getScaleOverLifetime() const
	{
		return mScaleOverLifetime;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool C_ParticleEffect::getEmitFromShell() const
	{
		return mEmitFromShell;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool C_ParticleEffect::getAdditive() const
	{
		return mAdditive;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool C_ParticleEffect::getBillbiarding() const
	{
		return mBillboarding;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool C_ParticleEffect::getGradienting() const
	{
		return mGradienting;
	}
	//////////////////////////////////////////////////////////////////////////////
	C_Vector3 C_ParticleEffect::getPos() const
	{
		return mPos;
	}
	//////////////////////////////////////////////////////////////////////////////
	C_Vector3 C_ParticleEffect::getMinVelocity() const
	{
		return mMinVelocity;
	}
	//////////////////////////////////////////////////////////////////////////////
	C_Vector3 C_ParticleEffect::getMaxVelocity() const
	{
		return mMaxVelocity;
	}
	//////////////////////////////////////////////////////////////////////////////
	C_Vector3 C_ParticleEffect::getMinAcceleration() const
	{
		return mMinAcceleration;
	}
	//////////////////////////////////////////////////////////////////////////////
	C_Vector3 C_ParticleEffect::getMaxAcceleration() const
	{
		return mMaxAcceleration;
	}
	//////////////////////////////////////////////////////////////////////////////
	C_Vector3 C_ParticleEffect::getConstantForce() const
	{
		return mConstantForce;
	}
	//////////////////////////////////////////////////////////////////////////////
	C_Vector2 C_ParticleEffect::getParticleSize() const
	{
		return mParticleSize;
	}
	//////////////////////////////////////////////////////////////////////////////
	C_Vector2 C_ParticleEffect::getStartSize() const
	{
		return mStartSize;
	}
	//////////////////////////////////////////////////////////////////////////////
	C_Vector2 C_ParticleEffect::getSubUV() const
	{
		return mSubUV;
	}
	//////////////////////////////////////////////////////////////////////////////
	C_Vector2 C_ParticleEffect::getFinalSize() const
	{
		return mFinalSize;
	}
	//////////////////////////////////////////////////////////////////////////////
	C_Vector4 C_ParticleEffect::getStartColor() const
	{
		return mStartColor;;
	}
	//////////////////////////////////////////////////////////////////////////////
	C_Vector4 C_ParticleEffect::getFinalColor() const
	{
		return mFinalColor;
	}
	//////////////////////////////////////////////////////////////////////////////
	C_Vector3 C_ParticleEffect::getBoxShapeSize() const
	{
		return mBoxShapeSize;
	}
	//////////////////////////////////////////////////////////////////////////////
	float C_ParticleEffect::getMinTimeToLive() const
	{
		return mMinTimeToLive;
	}
	//////////////////////////////////////////////////////////////////////////////
	float C_ParticleEffect::getMaxTimeToLive() const
	{
		return mMaxTimeToLive;
	}
	//////////////////////////////////////////////////////////////////////////////
	float C_ParticleEffect::getMinRotation() const
	{
		return mMinRotation;
	}
	//////////////////////////////////////////////////////////////////////////////
	float C_ParticleEffect::getMaxRotation() const
	{
		return mMaxRotation;
	}
	//////////////////////////////////////////////////////////////////////////////
	float C_ParticleEffect::getMinRotationSpeed() const
	{
		return mMinRotationSpeed;
	}
	//////////////////////////////////////////////////////////////////////////////
	float C_ParticleEffect::getMaxRotationSpeed() const
	{
		return mMaxRotationSpeed;
	}
	//////////////////////////////////////////////////////////////////////////////
	float C_ParticleEffect::getNoiseStrength() const
	{
		return mNoiseStrength;
	}
	//////////////////////////////////////////////////////////////////////////////
	unsigned int C_ParticleEffect::getNoiseOctaves() const
	{
		return mNoiseOctaves;
	}
	//////////////////////////////////////////////////////////////////////////////
	float C_ParticleEffect::getNoiseLacunarity() const
	{
		return mNoiseLacunarity;
	}
	//////////////////////////////////////////////////////////////////////////////
	float C_ParticleEffect::getNoisePersistence() const
	{
		return mNoisePersistence;
	}
	//////////////////////////////////////////////////////////////////////////////
	float C_ParticleEffect::getNoiseFrequency() const
	{
		return mNoiseFrequency;
	}
	//////////////////////////////////////////////////////////////////////////////
	float C_ParticleEffect::getNoiseAmplitude() const
	{
		return mNoiseAmplitude;
	}
	//////////////////////////////////////////////////////////////////////////////
	float C_ParticleEffect::getEmitRate() const
	{
		return mEmitRate;
	}
	//////////////////////////////////////////////////////////////////////////////
	int C_ParticleEffect::getTransformation() const
	{
		return mParticleTransformation;
	}
	//////////////////////////////////////////////////////////////////////////////
	int C_ParticleEffect::getParticleShape() const
	{
		return mParticleShape;
	}
	//////////////////////////////////////////////////////////////////////////////
	float C_ParticleEffect::getParticleShapeRadius() const
	{
		return mParticleShapeRadius;
	}
	//////////////////////////////////////////////////////////////////////////////
	int C_ParticleEffect::getSortMode() const
	{
		return mSortMode;
	}
	//////////////////////////////////////////////////////////////////////////////
	int C_ParticleEffect::getSubUVMode() const
	{
		return mSubUVMode;
	}
	//////////////////////////////////////////////////////////////////////////////
	float C_ParticleEffect::getSubUVCycles() const
	{
		return mSubUVCycles;
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
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

		if (!serializer.setVector2("SubUV", mSubUV, { "X", "Y" }))
		{ C_Log::error("Can't save Particles sub UV: " + aFile); return false; }

		if (!serializer.setVector4("StartColor", mStartColor, { "R", "G", "B", "A" }))
		{ C_Log::error("Can't save Particles start color: ", aFile); return false; }

		if (!serializer.setVector4("FinalColor", mFinalColor, { "R", "G", "B", "A" }))
		{ C_Log::error("Can't save Particles final color: ", aFile); return false; }

		if (!serializer.setVector3("BoxShapeSize", mBoxShapeSize, { "X", "Y", "Z" }))
		{ C_Log::error("Can't save Particles box shape size: ", aFile); return false; }

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

		if (!serializer.setFloat("NoiseStrength", mNoiseStrength))
		{ C_Log::error("Can't save Particles noise strength: " + aFile); return false; }

		if (!serializer.setInt("NoiseOctaves", mNoiseOctaves))
		{ C_Log::error("Can't save Particles noise octaves: " + aFile); return false; }

		if (!serializer.setFloat("NoiseLacunarity", mNoiseLacunarity))
		{ C_Log::error("Can't save Particles noise lacunarity: " + aFile); return false; }	

		if (!serializer.setFloat("NoisePersistence", mNoisePersistence))
		{ C_Log::error("Can't save Particles noise persistence: " + aFile); return false; }	

		if (!serializer.setFloat("NoiseFrequency", mNoiseFrequency))
		{ C_Log::error("Can't save Particles noise frequency: " + aFile); return false; }

		if (!serializer.setFloat("NoiseAmplitude", mNoiseAmplitude))
		{ C_Log::error("Can't save Particles noise amplitude: " + aFile); return false; }

		if (!serializer.setFloat("EmitRate", mEmitRate))
		{ C_Log::error("Can't save Particles emit rate: " + aFile); return false; }

		if (!serializer.setInt("Transformation", mParticleTransformation))
		{ C_Log::error("Can't save Particles transformation: " + aFile); return false; }

		if (!serializer.setInt("Shape", mParticleShape))
		{ C_Log::error("Can't save Particles shape: " + aFile); return false; }

		if (!serializer.setFloat("ShapeRadius", mParticleShapeRadius))
		{ C_Log::error("Can't save Particles shape radius: " + aFile); return false; }

		if (!serializer.setInt("SortMode", mSortMode))
		{ C_Log::error("Can't save Particles sort mode: " + aFile); return false; }

		if (!serializer.setInt("SubUVMode", mSubUVMode))
		{ C_Log::error("Can't save Particles sub UV mode: " + aFile); return false; }

		if (!serializer.setFloat("SubUVCycles", mSubUVCycles))
		{ C_Log::error("Can't save Particles sub UV cycles: " + aFile); return false; }

		if (!serializer.save())
		{ C_Log::error("Can't save Particle Effect: " + aFile); return false; }


		C_Log::success("Particle Effect saved: " + aFile);

		return true;
	}
	//////////////////////////////////////////////////////////////////////////////
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

		if (!serializer.getVector2("SubUV", &mSubUV, { "X", "Y" }))
		{ C_Log::error("Can't load Particles sub UV: " + aFile); return false; }

		if (!serializer.getVector4("StartColor", &mStartColor, { "R", "G", "B", "A"}))
		{ C_Log::error("Can't load Particles start color: " + aFile); return false; }

		if (!serializer.getVector4("FinalColor", &mFinalColor, { "R", "G", "B", "A" }))
		{ C_Log::error("Can't load Particles final color: " + aFile); return false; }

		if (!serializer.getVector3("BoxShapeSize", &mBoxShapeSize, { "X", "Y", "Z" }))
		{ C_Log::error("Can't load Particles box shape size: " + aFile); return false; }

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

		if (!serializer.getFloat("NoiseStrength", &mNoiseStrength))
		{ C_Log::error("Can't load Particles noise strength: " + aFile); return false; }

		if (!serializer.getInt("NoiseOctaves", (int*)&mNoiseOctaves))
		{ C_Log::error("Can't load Particles noise octaves: " + aFile); return false; }

		if (!serializer.getFloat("NoiseLacunarity", &mNoiseLacunarity))
		{ C_Log::error("Can't load Particles noise lacunarity: " + aFile); return false; }

		if (!serializer.getFloat("NoisePersistence", &mNoisePersistence))
		{ C_Log::error("Can't load Particles noise persistence: " + aFile); return false; }

		if (!serializer.getFloat("NoiseFrequency", &mNoiseFrequency))
		{ C_Log::error("Can't load Particles noise frequency: " + aFile); return false; }

		if (!serializer.getFloat("NoiseAmplitude", &mNoiseAmplitude))
		{ C_Log::error("Can't load Particles noise amplitude: " + aFile); return false; }

		if (!serializer.getFloat("EmitRate", &mEmitRate))
		{ C_Log::error("Can't load Particles emit rate: " + aFile); return false; }

		if (!serializer.getInt("Transformation", &mParticleTransformation))
		{ C_Log::error("Can't load Particles transformation: " + aFile); return false; }

		if (!serializer.getInt("Shape", &mParticleShape))
		{ C_Log::error("Can't load Particles shape: " + aFile); return false; }

		if (!serializer.getFloat("ShapeRadius", &mParticleShapeRadius))
		{ C_Log::error("Can't load Particles shape radius: " + aFile); return false; }

		if (!serializer.getInt("SortMode", &mSortMode))
		{ C_Log::error("Can't load Particles sort mode: " + aFile); return false; }

		if (!serializer.getInt("SubUVMode", &mSubUVMode))
		{ C_Log::error("Can't load Particles sub UV mode: " + aFile); return false; }

		if (!serializer.getFloat("SubUVCycles", &mSubUVCycles))
		{ C_Log::error("Can't load Particles sub UV cycles: " + aFile); return false; }

		C_Log::success("Particle Effect loaded: " + aFile);

		return true;
	}
	//////////////////////////////////////////////////////////////////////////////
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
