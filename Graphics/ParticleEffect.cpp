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
	ParticleEffect::ParticleEffect()
	{

	}
	//////////////////////////////////////////////////////////////////////////////
	ParticleEffect::ParticleEffect(std::string aFile)
	{
		load(aFile);
	}
	//////////////////////////////////////////////////////////////////////////////
	ParticleEffect::ParticleEffect(std::string aFile, Material* aMaterial)
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
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	Material* ParticleEffect::getMaterial() const
	{
		return mMaterial;
	}
	//////////////////////////////////////////////////////////////////////////////
	Vector3 ParticleEffect::getPos() const
	{
		return mPos;
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	bool ParticleEffect::saveToXML(std::string aFile) const
	{
		/*Serializer::SerializerXML serializer;

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


		Log::success("Particle Effect saved: " + aFile);*/

		return true;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool ParticleEffect::loadFromXML(std::string aFile)
	{
		Serializer::SerializerXML serializer;

		if (!serializer.read(aFile, "ParticleEffect"))
		{ Log::error("Can't load Particle Effect: " + aFile); return false; }

		ParticleRequired tRequired;
		ParticleEmit tEmit;
		ParticleLocation tLocation;
		ParticleLifetime tLifetime;
		ParticleInitialRotation tInitialRotation;
		ParticleInitialVelocity tInitialVelocity;
		ParticleInitialAcceleration tInitialAcceleration;
		ParticleInitialSize tInitialSize;
		ParticleSizeOverLife tSizeOverLife;
		ParticleInitialColor tInitialColor;
		ParticleColorOverLife tColorOverLife;
		ParticleNoise tNoise;
		ParticleSubUV tSubUV;

		serializer.getSubBool({ "Required", "Visible" }, &tRequired.Visible);
		serializer.getSubBool({ "Required", "AdditiveBlending" }, &tRequired.AdditiveBlending);
		serializer.getSubBool({ "Required", "Billboarding" }, &tRequired.Billboarding);
		serializer.getSubInt({ "Required", "Transformation" }, (int*)&tRequired.Transformation);
		serializer.getSubInt({ "Required", "Transformation" }, (int*)&tRequired.SortMode);

		serializer.getSubBool({ "Emit", "Emitting" }, &tEmit.Emitting);
		serializer.getSubInt({ "Emit", "Count" }, &tEmit.Count);
		serializer.getSubFloat({ "Emit", "EmitRate" }, &tEmit.EmitRate);

		serializer.getSubBool({ "Location", "EmitFromShell" }, &tLocation.EmitFromShell);
		serializer.getSubInt({ "Location", "Shape" }, (int*)&tLocation.Shape);
		serializer.getSubFloat({ "Location", "Radius" }, &tLocation.Radius);
		serializer.getSubVector3({ "Location", "Size" }, &tLocation.Size, { "X", "Y", "Z" });

		serializer.getSubFloat({ "Lifetime", "Min" }, &tLifetime.Min);
		serializer.getSubFloat({ "Lifetime", "Max" }, &tLifetime.Max);

		serializer.getSubFloat({ "InitialRotation", "Min" }, &tInitialRotation.Min);
		serializer.getSubFloat({ "InitialRotation", "Max" }, &tInitialRotation.Max);
		serializer.getSubFloat({ "InitialRotation", "MinVelocity" }, &tInitialRotation.MinVelocity);
		serializer.getSubFloat({ "InitialRotation", "MaxVelocity" }, &tInitialRotation.MaxVelocity);

		serializer.getSubVector3({ "InitialVelocity", "Min" }, &tInitialVelocity.Min, { "X", "Y", "Z" });
		serializer.getSubVector3({ "InitialVelocity", "Max" }, &tInitialVelocity.Max, { "X", "Y", "Z" });

		serializer.getSubVector3({ "InitialAcceleration", "Min" }, &tInitialAcceleration.Min, {  "X", "Y", "Z" });
		serializer.getSubVector3({ "InitialAcceleration", "Max" }, &tInitialAcceleration.Max, {  "X", "Y", "Z" });

		serializer.getSubVector3({ "InitialSize", "Min" }, &tInitialSize.Min, { "X", "Y", "Z" });
		serializer.getSubVector3({ "InitialSize", "Max" }, &tInitialSize.Max, { "X", "Y", "Z" });

		serializer.getSubBool({ "SizeOverLife", "Active" }, &tSizeOverLife.Active);
		serializer.getSubVector3({ "SizeOverLife", "MinStart" }, &tSizeOverLife.MinStart, { "X", "Y", "Z" });
		serializer.getSubVector3({ "SizeOverLife", "MaxStart" }, &tSizeOverLife.MaxStart, { "X", "Y", "Z" });
		serializer.getSubVector3({ "SizeOverLife", "MinFinal" }, &tSizeOverLife.MinFinal, { "X", "Y", "Z" });
		serializer.getSubVector3({ "SizeOverLife", "MaxFinal" }, &tSizeOverLife.MaxFinal, { "X", "Y", "Z" });

		serializer.getSubVector4({ "InitialColor", "Min" }, &tInitialColor.Min, { "R", "G", "B", "A" });
		serializer.getSubVector4({ "InitialColor", "Max" }, &tInitialColor.Max, { "R", "G", "B", "A" });

		serializer.getSubBool({ "ColorOverLife", "Active" }, &tColorOverLife.Active);
		serializer.getSubVector4({ "ColorOverLife", "MinStart" }, &tColorOverLife.MinStart, { "R", "G", "B", "A" });
		serializer.getSubVector4({ "ColorOverLife", "MaxStart" }, &tColorOverLife.MaxStart, { "R", "G", "B", "A" });
		serializer.getSubVector4({ "ColorOverLife", "MinFinal" }, &tColorOverLife.MinFinal, { "R", "G", "B", "A" });
		serializer.getSubVector4({ "ColorOverLife", "MaxFinal" }, &tColorOverLife.MaxFinal, { "R", "G", "B", "A" });

		serializer.getSubBool({ "Noise", "Active" }, &tNoise.Active);
		serializer.getSubFloat({ "Noise", "Strength" }, &tNoise.Strength);
		serializer.getSubInt({ "Noise", "Octaves" }, &tNoise.Octaves);
		serializer.getSubFloat({ "Noise", "Lacunarity" }, &tNoise.Lacunarity);
		serializer.getSubFloat({ "Noise", "Persistence" }, &tNoise.Persistence);
		serializer.getSubFloat({ "Noise", "Frequency" }, &tNoise.Frequency);
		serializer.getSubFloat({ "Noise", "Amplitude" }, &tNoise.Amplitude);

		serializer.getSubInt({ "SubUV", "Mode" }, (int*)&tSubUV.Mode);
		serializer.getSubInt({ "SubUV", "Horizontal" }, &tSubUV.Horizontal);
		serializer.getSubInt({ "SubUV", "Vertical" }, &tSubUV.Vertical);
		serializer.getSubFloat({ "SubUV", "Cycles" }, &tSubUV.Cycles);

		Log::success("Particle Effect loaded: " + aFile);

		Required = tRequired;
		Emit = tEmit;
		Location = tLocation;
		Lifetime = tLifetime;
		InitialRotation = tInitialRotation;
		InitialVelocity = tInitialVelocity;
		InitialAcceleration = tInitialAcceleration;
		InitialSize = tInitialSize;
		SizeOverLife = tSizeOverLife;
		InitialColor = tInitialColor;
		ColorOverLife = tColorOverLife;
		Noise = tNoise;
		SubUV = tSubUV;

		return true;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool ParticleEffect::load(std::string aFile)
	{
		if (aFile.find_last_of(".cxpar") != std::string::npos)
		{
			return loadFromXML(aFile);
		} else return false;
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	ParticleEffect::~ParticleEffect()
	{

	}

}
