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
	ParticleEffect::ParticleEffect() :
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
		mNoise(false),
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
	ParticleEffect::ParticleEffect(std::string aFile) :
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
		mNoise(false),
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
	ParticleEffect::ParticleEffect(std::string aFile, Material* aMaterial) :
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
		mNoise(false),
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
	void ParticleEffect::setMaterial(const Material* aMaterial)
	{
		mMaterial = const_cast<Material*>(aMaterial);
	}
	//////////////////////////////////////////////////////////////////////////////
	void ParticleEffect::setParticlesCount(const int aParticlesCount)
	{
		mParticlesCount = static_cast<int>(aParticlesCount);
	}
	//////////////////////////////////////////////////////////////////////////////
	void ParticleEffect::setVisible(const bool aVisible)
	{
		mVisible = static_cast<bool>(aVisible);
	}
	//////////////////////////////////////////////////////////////////////////////
	void ParticleEffect::setScaleOverLifetime(const bool aA)
	{
		mScaleOverLifetime = static_cast<bool>(aA);
	}
	//////////////////////////////////////////////////////////////////////////////
	void ParticleEffect::setEmitFromShell(const bool aA)
	{
		mEmitFromShell = static_cast<bool>(aA);
	}
	//////////////////////////////////////////////////////////////////////////////
	void ParticleEffect::setAdditive(const bool aA)
	{
		mAdditive = static_cast<bool>(aA);
	}
	//////////////////////////////////////////////////////////////////////////////
	void ParticleEffect::setBillboarding(const bool aA)
	{
		mBillboarding = static_cast<bool>(aA);
	}
	//////////////////////////////////////////////////////////////////////////////
	void ParticleEffect::setGradienting(const bool aA)
	{
		mGradienting = static_cast<bool>(aA);
	}
	//////////////////////////////////////////////////////////////////////////////
	void ParticleEffect::setPos(const Vector3 aPos)
	{
		mPos = static_cast<Vector3>(aPos);
	}
	//////////////////////////////////////////////////////////////////////////////
	void ParticleEffect::addPos(const Vector3 aPos)
	{
		mPos += static_cast<Vector3>(aPos);
	}
	//////////////////////////////////////////////////////////////////////////////
	void ParticleEffect::setMinVelocity(const Vector3 aMinVelocity)
	{
		mMinVelocity = static_cast<Vector3>(aMinVelocity);
	}
	//////////////////////////////////////////////////////////////////////////////
	void ParticleEffect::setMaxVelocity(const Vector3 aMaxVelocity)
	{
		mMaxVelocity = static_cast<Vector3>(aMaxVelocity);
	}
	//////////////////////////////////////////////////////////////////////////////
	void ParticleEffect::setMinAcceleration(const Vector3 aMinAcceleration)
	{
		mMinAcceleration = static_cast<Vector3>(aMinAcceleration);
	}
	//////////////////////////////////////////////////////////////////////////////
	void ParticleEffect::setMaxAcceleration(const Vector3 aMaxAcceleration)
	{
		mMaxAcceleration = static_cast<Vector3>(aMaxAcceleration);
	}
	//////////////////////////////////////////////////////////////////////////////
	void ParticleEffect::setConstantForce(const Vector3 aConstantForce)
	{
		mConstantForce = static_cast<Vector3>(aConstantForce);
	}
	//////////////////////////////////////////////////////////////////////////////
	void ParticleEffect::setParticleSize(const Vector2 aParticleSize)
	{
		mParticleSize = static_cast<Vector2>(aParticleSize);
	}
	//////////////////////////////////////////////////////////////////////////////
	void ParticleEffect::setStartSize(const Vector2 aStartSize)
	{
		mStartSize = static_cast<Vector2>(aStartSize);
	}
	//////////////////////////////////////////////////////////////////////////////
	void ParticleEffect::setFinalSize(const Vector2 aFinalSize)
	{
		mFinalSize = static_cast<Vector2>(aFinalSize);
	}
	//////////////////////////////////////////////////////////////////////////////
	void ParticleEffect::setSubUV(const Vector2 aSubUV)
	{
		mSubUV = static_cast<Vector2>(aSubUV);
	}
	//////////////////////////////////////////////////////////////////////////////
	void ParticleEffect::setStartColor(const Vector4 aStartColor)
	{
		mStartColor = static_cast<Vector4>(aStartColor);
	}
	//////////////////////////////////////////////////////////////////////////////
	void ParticleEffect::setFinalColor(const Vector4 aFinalColor)
	{
		mFinalColor = static_cast<Vector4>(aFinalColor);
	}
	//////////////////////////////////////////////////////////////////////////////
	void ParticleEffect::setBoxShapeSize(const Vector3 aBoxShapeSize)
	{
		mBoxShapeSize = static_cast<Vector3>(aBoxShapeSize);
	}
	//////////////////////////////////////////////////////////////////////////////
	void ParticleEffect::setMinTimeToLive(const float aMinTimeToLive)
	{
		mMinTimeToLive = static_cast<float>(aMinTimeToLive);
	}
	//////////////////////////////////////////////////////////////////////////////
	void ParticleEffect::setMaxTimeToLive(const float aMaxTimeToLive)
	{
		mMaxTimeToLive = static_cast<float>(aMaxTimeToLive);
	}
	//////////////////////////////////////////////////////////////////////////////
	void ParticleEffect::setMinRotation(const float aMinRotation)
	{
		mMinRotation = static_cast<float>(aMinRotation);
	}
	//////////////////////////////////////////////////////////////////////////////
	void ParticleEffect::setMaxRotation(const float aMaxRotation)
	{
		mMaxRotation = static_cast<float>(aMaxRotation);
	}
	//////////////////////////////////////////////////////////////////////////////
	void ParticleEffect::setMinRotationSpeed(const float aMinRotationSpeed)
	{
		mMinRotationSpeed = static_cast<float>(aMinRotationSpeed);
	}
	//////////////////////////////////////////////////////////////////////////////
	void ParticleEffect::setMaxRotationSpeed(const float aMaxRotationSpeed)
	{
		mMaxRotationSpeed = static_cast<float>(aMaxRotationSpeed);
	}
	//////////////////////////////////////////////////////////////////////////////
	void ParticleEffect::setNoise(const bool aNoise)
	{
		mNoise = static_cast<bool>(aNoise);
	}
	//////////////////////////////////////////////////////////////////////////////
	void ParticleEffect::setNoiseStrength(const float aNoiseStrength)
	{
		mNoiseStrength = static_cast<float>(aNoiseStrength);
	}
	//////////////////////////////////////////////////////////////////////////////
	void ParticleEffect::setNoiseOctaves(const unsigned int aNoiseOctaves)
	{
		mNoiseOctaves = static_cast<unsigned int>(aNoiseOctaves);
	}
	//////////////////////////////////////////////////////////////////////////////
	void ParticleEffect::setNoiseLacunarity(const float aNoiseLacunarity)
	{
		mNoiseLacunarity = static_cast<float>(aNoiseLacunarity);
	}
	//////////////////////////////////////////////////////////////////////////////
	void ParticleEffect::setNoisePersistence(const float aNoisePersistence)
	{
		mNoisePersistence = static_cast<float>(aNoisePersistence);
	}
	//////////////////////////////////////////////////////////////////////////////
	void ParticleEffect::setNoiseFrequency(const float aNoiseFrequency)
	{
		mNoiseFrequency = static_cast<float>(aNoiseFrequency);
	}
	//////////////////////////////////////////////////////////////////////////////
	void ParticleEffect::setNoiseAmplitude(const float aNoiseAmplitude)
	{
		mNoiseAmplitude = static_cast<float>(aNoiseAmplitude);
	}
	//////////////////////////////////////////////////////////////////////////////
	void ParticleEffect::setEmitRate(const float aEmitRate)
	{
		mEmitRate = static_cast<float>(aEmitRate);
	}
	//////////////////////////////////////////////////////////////////////////////
	void ParticleEffect::setTransformation(const E_PARTICLE_TRANSFORMATION aParticleTransformation)
	{
		mParticleTransformation = static_cast<int>(aParticleTransformation);
	}
	//////////////////////////////////////////////////////////////////////////////
	void ParticleEffect::setParticleShape(const E_PARTICLE_SHAPE aParticleShape)
	{
		mParticleShape = static_cast<int>(aParticleShape);
	}
	//////////////////////////////////////////////////////////////////////////////
	void ParticleEffect::setParticleShapeRadius(const float aRadius)
	{
		mParticleShapeRadius = static_cast<float>(aRadius);
	}
	//////////////////////////////////////////////////////////////////////////////
	void ParticleEffect::setSortMode(const E_PARTICLE_SORT_MODE aSortMode)
	{
		mSortMode = static_cast<int>(aSortMode);
	}
	//////////////////////////////////////////////////////////////////////////////
	void ParticleEffect::setSubUVMode(const E_PARTICLE_SUB_UV_MODE aSubUVMode)
	{
		mSubUVMode = static_cast<int>(aSubUVMode);
	}
	//////////////////////////////////////////////////////////////////////////////
	void ParticleEffect::setSubUVCycles(const float aSubUVCycles)
	{
		mSubUVCycles = static_cast<float>(aSubUVCycles);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	Material* ParticleEffect::getMaterial() const
	{
		return mMaterial;
	}
	//////////////////////////////////////////////////////////////////////////////
	int ParticleEffect::getParticlesCount() const
	{
		return mParticlesCount;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool ParticleEffect::getVisible() const
	{
		return mVisible;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool ParticleEffect::getScaleOverLifetime() const
	{
		return mScaleOverLifetime;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool ParticleEffect::getEmitFromShell() const
	{
		return mEmitFromShell;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool ParticleEffect::getAdditive() const
	{
		return mAdditive;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool ParticleEffect::getBillbiarding() const
	{
		return mBillboarding;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool ParticleEffect::getGradienting() const
	{
		return mGradienting;
	}
	//////////////////////////////////////////////////////////////////////////////
	Vector3 ParticleEffect::getPos() const
	{
		return mPos;
	}
	//////////////////////////////////////////////////////////////////////////////
	Vector3 ParticleEffect::getMinVelocity() const
	{
		return mMinVelocity;
	}
	//////////////////////////////////////////////////////////////////////////////
	Vector3 ParticleEffect::getMaxVelocity() const
	{
		return mMaxVelocity;
	}
	//////////////////////////////////////////////////////////////////////////////
	Vector3 ParticleEffect::getMinAcceleration() const
	{
		return mMinAcceleration;
	}
	//////////////////////////////////////////////////////////////////////////////
	Vector3 ParticleEffect::getMaxAcceleration() const
	{
		return mMaxAcceleration;
	}
	//////////////////////////////////////////////////////////////////////////////
	Vector3 ParticleEffect::getConstantForce() const
	{
		return mConstantForce;
	}
	//////////////////////////////////////////////////////////////////////////////
	Vector2 ParticleEffect::getParticleSize() const
	{
		return mParticleSize;
	}
	//////////////////////////////////////////////////////////////////////////////
	Vector2 ParticleEffect::getStartSize() const
	{
		return mStartSize;
	}
	//////////////////////////////////////////////////////////////////////////////
	Vector2 ParticleEffect::getSubUV() const
	{
		return mSubUV;
	}
	//////////////////////////////////////////////////////////////////////////////
	Vector2 ParticleEffect::getFinalSize() const
	{
		return mFinalSize;
	}
	//////////////////////////////////////////////////////////////////////////////
	Vector4 ParticleEffect::getStartColor() const
	{
		return mStartColor;;
	}
	//////////////////////////////////////////////////////////////////////////////
	Vector4 ParticleEffect::getFinalColor() const
	{
		return mFinalColor;
	}
	//////////////////////////////////////////////////////////////////////////////
	Vector3 ParticleEffect::getBoxShapeSize() const
	{
		return mBoxShapeSize;
	}
	//////////////////////////////////////////////////////////////////////////////
	float ParticleEffect::getMinTimeToLive() const
	{
		return mMinTimeToLive;
	}
	//////////////////////////////////////////////////////////////////////////////
	float ParticleEffect::getMaxTimeToLive() const
	{
		return mMaxTimeToLive;
	}
	//////////////////////////////////////////////////////////////////////////////
	float ParticleEffect::getMinRotation() const
	{
		return mMinRotation;
	}
	//////////////////////////////////////////////////////////////////////////////
	float ParticleEffect::getMaxRotation() const
	{
		return mMaxRotation;
	}
	//////////////////////////////////////////////////////////////////////////////
	float ParticleEffect::getMinRotationSpeed() const
	{
		return mMinRotationSpeed;
	}
	//////////////////////////////////////////////////////////////////////////////
	float ParticleEffect::getMaxRotationSpeed() const
	{
		return mMaxRotationSpeed;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool ParticleEffect::getNoise() const
	{
		return mNoise;
	}
	//////////////////////////////////////////////////////////////////////////////
	float ParticleEffect::getNoiseStrength() const
	{
		return mNoiseStrength;
	}
	//////////////////////////////////////////////////////////////////////////////
	unsigned int ParticleEffect::getNoiseOctaves() const
	{
		return mNoiseOctaves;
	}
	//////////////////////////////////////////////////////////////////////////////
	float ParticleEffect::getNoiseLacunarity() const
	{
		return mNoiseLacunarity;
	}
	//////////////////////////////////////////////////////////////////////////////
	float ParticleEffect::getNoisePersistence() const
	{
		return mNoisePersistence;
	}
	//////////////////////////////////////////////////////////////////////////////
	float ParticleEffect::getNoiseFrequency() const
	{
		return mNoiseFrequency;
	}
	//////////////////////////////////////////////////////////////////////////////
	float ParticleEffect::getNoiseAmplitude() const
	{
		return mNoiseAmplitude;
	}
	//////////////////////////////////////////////////////////////////////////////
	float ParticleEffect::getEmitRate() const
	{
		return mEmitRate;
	}
	//////////////////////////////////////////////////////////////////////////////
	int ParticleEffect::getTransformation() const
	{
		return mParticleTransformation;
	}
	//////////////////////////////////////////////////////////////////////////////
	int ParticleEffect::getParticleShape() const
	{
		return mParticleShape;
	}
	//////////////////////////////////////////////////////////////////////////////
	float ParticleEffect::getParticleShapeRadius() const
	{
		return mParticleShapeRadius;
	}
	//////////////////////////////////////////////////////////////////////////////
	int ParticleEffect::getSortMode() const
	{
		return mSortMode;
	}
	//////////////////////////////////////////////////////////////////////////////
	int ParticleEffect::getSubUVMode() const
	{
		return mSubUVMode;
	}
	//////////////////////////////////////////////////////////////////////////////
	float ParticleEffect::getSubUVCycles() const
	{
		return mSubUVCycles;
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	bool ParticleEffect::saveToXML(std::string aFile) const
	{
		Serializer::SerializerXML serializer;

		if (!serializer.write(aFile, "ParticleEffect"))
		{ Log::error("Can't save Particle Effect: " + aFile); return false; }

		if (!serializer.setInt("Count", mParticlesCount))
		{ Log::error("Can't save Particles count: " + aFile); return false; }

		if (!serializer.setBool("Visible", mVisible))
		{ Log::error("Can't save Particles visible: " + aFile); return false; }

		if (!serializer.setBool("ScaleOL", mScaleOverLifetime))
		{ Log::error("Can't save Particles scale over lifetime: " + aFile); return false; }

		if (!serializer.setBool("EmitFromShell", mEmitFromShell))
		{ Log::error("Can't save Particles emit from shell: " + aFile); return false; }

		if (!serializer.setBool("AdditiveBlending", mAdditive))
		{ Log::error("Can't save Particles additive blending: " + aFile); return false; }

		if (!serializer.setBool("Billboarding", mBillboarding))
		{ Log::error("Can't save Particles billboarding: " + aFile); return false; }

		if (!serializer.setBool("Gradienting", mGradienting))
		{ Log::error("Can't save Particles gradienting: " + aFile); return false; }

		if (!serializer.setVector3("MinVelocity", mMinVelocity, { "X", "Y", "Z" }))
		{ Log::error("Can't save Particles min velocity: " + aFile); return false; }

		if (!serializer.setVector3("MaxVelocity", mMaxVelocity, { "X", "Y", "Z" }))
		{ Log::error("Can't save Particles max velocity: " + aFile); return false; }

		if (!serializer.setVector3("MinAcceleration", mMinAcceleration, { "X", "Y", "Z" }))
		{ Log::error("Can't save Particles min acceleration: " + aFile); return false; }

		if (!serializer.setVector3("MaxAcceleration", mMaxAcceleration, { "X", "Y", "Z" }))
		{ Log::error("Can't save Particles max acceleration: " + aFile); return false; }

		if (!serializer.setVector3("ConstForce", mConstantForce, { "X", "Y", "Z" }))
		{ Log::error("Can't save Particles constant force: " + aFile); return false; }

		if (!serializer.setVector2("PartSize", mParticleSize, { "X", "Y" }))
		{ Log::error("Can't save Particles size: " + aFile); return false; }

		if (!serializer.setVector2("StartSize", mStartSize, { "X", "Y" }))
		{ Log::error("Can't save Particles start size: " + aFile); return false; }

		if (!serializer.setVector2("FinalSize", mFinalSize, { "X", "Y" }))
		{ Log::error("Can't save Particles final size: " + aFile); return false; }

		if (!serializer.setVector2("SubUV", mSubUV, { "X", "Y" }))
		{ Log::error("Can't save Particles sub UV: " + aFile); return false; }

		if (!serializer.setVector4("StartColor", mStartColor, { "R", "G", "B", "A" }))
		{ Log::error("Can't save Particles start color: ", aFile); return false; }

		if (!serializer.setVector4("FinalColor", mFinalColor, { "R", "G", "B", "A" }))
		{ Log::error("Can't save Particles final color: ", aFile); return false; }

		if (!serializer.setVector3("BoxShapeSize", mBoxShapeSize, { "X", "Y", "Z" }))
		{ Log::error("Can't save Particles box shape size: ", aFile); return false; }

		if (!serializer.setFloat("MinTTL", mMinTimeToLive))
		{ Log::error("Can't save Particles min TTL: " + aFile); return false; }

		if (!serializer.setFloat("MaxTTL", mMaxTimeToLive))
		{ Log::error("Can't save Particles max TTL: " + aFile); return false; }

		if (!serializer.setFloat("MinRotation", mMinRotation))
		{ Log::error("Can't save Particles min rotation: " + aFile); return false; }

		if (!serializer.setFloat("MaxRotation", mMaxRotation))
		{ Log::error("Can't save Particles max rotation: " + aFile); return false; }

		if (!serializer.setFloat("MinRotationSpeed", mMinRotationSpeed))
		{ Log::error("Can't save Particles min rotation speed: " + aFile); return false; }

		if (!serializer.setFloat("MaxRotationSpeed", mMaxRotationSpeed))
		{ Log::error("Can't save Particles max rotation speed: " + aFile); return false; }

		if (!serializer.setBool("Noise", mNoise))
		{ Log::error("Can't save Particles noise: " + aFile); return false; }

		if (!serializer.setFloat("NoiseStrength", mNoiseStrength))
		{ Log::error("Can't save Particles noise strength: " + aFile); return false; }

		if (!serializer.setInt("NoiseOctaves", mNoiseOctaves))
		{ Log::error("Can't save Particles noise octaves: " + aFile); return false; }

		if (!serializer.setFloat("NoiseLacunarity", mNoiseLacunarity))
		{ Log::error("Can't save Particles noise lacunarity: " + aFile); return false; }	

		if (!serializer.setFloat("NoisePersistence", mNoisePersistence))
		{ Log::error("Can't save Particles noise persistence: " + aFile); return false; }	

		if (!serializer.setFloat("NoiseFrequency", mNoiseFrequency))
		{ Log::error("Can't save Particles noise frequency: " + aFile); return false; }

		if (!serializer.setFloat("NoiseAmplitude", mNoiseAmplitude))
		{ Log::error("Can't save Particles noise amplitude: " + aFile); return false; }

		if (!serializer.setFloat("EmitRate", mEmitRate))
		{ Log::error("Can't save Particles emit rate: " + aFile); return false; }

		if (!serializer.setInt("Transformation", mParticleTransformation))
		{ Log::error("Can't save Particles transformation: " + aFile); return false; }

		if (!serializer.setInt("Shape", mParticleShape))
		{ Log::error("Can't save Particles shape: " + aFile); return false; }

		if (!serializer.setFloat("ShapeRadius", mParticleShapeRadius))
		{ Log::error("Can't save Particles shape radius: " + aFile); return false; }

		if (!serializer.setInt("SortMode", mSortMode))
		{ Log::error("Can't save Particles sort mode: " + aFile); return false; }

		if (!serializer.setInt("SubUVMode", mSubUVMode))
		{ Log::error("Can't save Particles sub UV mode: " + aFile); return false; }

		if (!serializer.setFloat("SubUVCycles", mSubUVCycles))
		{ Log::error("Can't save Particles sub UV cycles: " + aFile); return false; }

		if (!serializer.save())
		{ Log::error("Can't save Particle Effect: " + aFile); return false; }


		Log::success("Particle Effect saved: " + aFile);

		return true;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool ParticleEffect::saveToJSON(std::string aFile) const
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
			Log::error("Can't save Particle Effect: " + aFile);
			return false;
		}

		o << std::setw(4) << j << std::endl;
		o.close();

		Log::success("Particle Effect saved: " + aFile);

		return true;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool ParticleEffect::loadFromXML(std::string aFile)
	{
		Serializer::SerializerXML serializer;

		if (!serializer.read(aFile, "ParticleEffect"))
		{ Log::error("Can't load Particle Effect: " + aFile); return false; }

		if (!serializer.getInt("Count", &mParticlesCount))
		{ Log::error("Can't load Particles count: " + aFile); return false; }

		if (!serializer.getBool("Visible", &mVisible))
		{ Log::error("Can't load Particles visible: " + aFile); return false; }

		if (!serializer.getBool("ScaleOL", &mScaleOverLifetime))
		{ Log::error("Can't load Particles scale over lifetime: " + aFile); return false; }

		if (!serializer.getBool("EmitFromShell", &mEmitFromShell))
		{ Log::error("Can't load Particles emit from shell: " + aFile); return false; }

		if (!serializer.getBool("AdditiveBlending", &mAdditive))
		{ Log::error("Can't load Particles additive blending: " + aFile); return false; }

		if (!serializer.getBool("Billboarding", &mBillboarding))
		{ Log::error("Can't load Particles billboarding: " + aFile); return false; }

		if (!serializer.getBool("Gradienting", &mGradienting))
		{ Log::error("Can't load Particles gradienting: " + aFile); return false; }

		if (!serializer.getVector3("MinVelocity", &mMinVelocity, { "X", "Y", "Z" }))
		{ Log::error("Can't load Particles min velocity: " + aFile); return false; }

		if (!serializer.getVector3("MaxVelocity", &mMaxVelocity, { "X", "Y", "Z" }))
		{ Log::error("Can't load Particles max velocity: " + aFile); return false; }

		if (!serializer.getVector3("MinAcceleration", &mMinAcceleration, { "X", "Y", "Z" }))
		{ Log::error("Can't load Particles min acceleration: " + aFile); return false; }

		if (!serializer.getVector3("MaxAcceleration", &mMaxAcceleration, { "X", "Y", "Z" }))
		{ Log::error("Can't load Particles max acceleration: " + aFile); return false; }

		if (!serializer.getVector3("ConstForce", &mConstantForce, { "X", "Y", "Z" }))
		{ Log::error("Can't load Particles constant force: " + aFile); return false; }

		if (!serializer.getVector2("PartSize", &mParticleSize, { "X", "Y"}))
		{ Log::error("Can't load Particles size: " + aFile); return false; }

		if (!serializer.getVector2("StartSize", &mStartSize, { "X", "Y" }))
		{ Log::error("Can't load Particles start size: " + aFile); return false; }

		if (!serializer.getVector2("FinalSize", &mFinalSize, { "X", "Y" }))
		{ Log::error("Can't load Particles final size: " + aFile); return false; }

		if (!serializer.getVector2("SubUV", &mSubUV, { "X", "Y" }))
		{ Log::error("Can't load Particles sub UV: " + aFile); return false; }

		if (!serializer.getVector4("StartColor", &mStartColor, { "R", "G", "B", "A"}))
		{ Log::error("Can't load Particles start color: " + aFile); return false; }

		if (!serializer.getVector4("FinalColor", &mFinalColor, { "R", "G", "B", "A" }))
		{ Log::error("Can't load Particles final color: " + aFile); return false; }

		if (!serializer.getVector3("BoxShapeSize", &mBoxShapeSize, { "X", "Y", "Z" }))
		{ Log::error("Can't load Particles box shape size: " + aFile); return false; }

		if (!serializer.getFloat("MinTTL", &mMinTimeToLive))
		{ Log::error("Can't load Particles min TTL: " + aFile); return false; }

		if (!serializer.getFloat("MaxTTL", &mMaxTimeToLive))
		{ Log::error("Can't load Particles max TTL: " + aFile); return false; }

		if (!serializer.getFloat("MinRotation", &mMinRotation))
		{ Log::error("Can't load Particles min rotation: " + aFile); return false; }

		if (!serializer.getFloat("MaxRotation", &mMaxRotation))
		{ Log::error("Can't load Particles max rotation: " + aFile); return false; }

		if (!serializer.getFloat("MinRotationSpeed", &mMinRotationSpeed))
		{ Log::error("Can't load Particles min rotation speed: " + aFile); return false; }

		if (!serializer.getFloat("MaxRotationSpeed", &mMaxRotationSpeed))
		{ Log::error("Can't load Particles max rotation speed: " + aFile); return false; }

		if (!serializer.getBool("Noise", &mNoise))
		{ Log::error("Can't load Particles noise: " + aFile); return false; }

		if (!serializer.getFloat("NoiseStrength", &mNoiseStrength))
		{ Log::error("Can't load Particles noise strength: " + aFile); return false; }

		if (!serializer.getInt("NoiseOctaves", (int*)&mNoiseOctaves))
		{ Log::error("Can't load Particles noise octaves: " + aFile); return false; }

		if (!serializer.getFloat("NoiseLacunarity", &mNoiseLacunarity))
		{ Log::error("Can't load Particles noise lacunarity: " + aFile); return false; }

		if (!serializer.getFloat("NoisePersistence", &mNoisePersistence))
		{ Log::error("Can't load Particles noise persistence: " + aFile); return false; }

		if (!serializer.getFloat("NoiseFrequency", &mNoiseFrequency))
		{ Log::error("Can't load Particles noise frequency: " + aFile); return false; }

		if (!serializer.getFloat("NoiseAmplitude", &mNoiseAmplitude))
		{ Log::error("Can't load Particles noise amplitude: " + aFile); return false; }

		if (!serializer.getFloat("EmitRate", &mEmitRate))
		{ Log::error("Can't load Particles emit rate: " + aFile); return false; }

		if (!serializer.getInt("Transformation", &mParticleTransformation))
		{ Log::error("Can't load Particles transformation: " + aFile); return false; }

		if (!serializer.getInt("Shape", &mParticleShape))
		{ Log::error("Can't load Particles shape: " + aFile); return false; }

		if (!serializer.getFloat("ShapeRadius", &mParticleShapeRadius))
		{ Log::error("Can't load Particles shape radius: " + aFile); return false; }

		if (!serializer.getInt("SortMode", &mSortMode))
		{ Log::error("Can't load Particles sort mode: " + aFile); return false; }

		if (!serializer.getInt("SubUVMode", &mSubUVMode))
		{ Log::error("Can't load Particles sub UV mode: " + aFile); return false; }

		if (!serializer.getFloat("SubUVCycles", &mSubUVCycles))
		{ Log::error("Can't load Particles sub UV cycles: " + aFile); return false; }

		Log::success("Particle Effect loaded: " + aFile);

		return true;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool ParticleEffect::loadFromJSON(std::string aFile)
	{
		std::ifstream i(aFile);
		if (i.is_open() == false)
		{
			Log::error("Can't load Particle Effect: " + aFile);
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

		Log::success("Particle Effect loaded: " + aFile);

		return true;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool ParticleEffect::load(std::string aFile)
	{
		if (aFile.find_last_of(".cxpar") != std::string::npos)
			return loadFromXML(aFile);
		else if (aFile.find_last_of(".cjpar") != std::string::npos)
			return loadFromJSON(aFile);
		else return false;
	}
	//////////////////////////////////////////////////////////////////////////////
	//Destructor
	ParticleEffect::~ParticleEffect() {}

}
