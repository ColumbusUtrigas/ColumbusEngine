/************************************************
*              ParticleEffect.cpp               *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#include <Graphics/Particles/ParticleEffect.h>

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
	void ParticleEffect::AddModule(ParticleModule* Module)
	{
		if (Module == nullptr) return;
		if (GetModule(Module->GetType()) != nullptr) return;
		Modules.push_back(Module);
	}
	//////////////////////////////////////////////////////////////////////////////
	ParticleModule* ParticleEffect::GetModule(ParticleModuleType Type) const
	{
		for (auto Module : Modules)
		{
			if (Module != nullptr)
			{
				if (Module->GetType() == Type)
				{
					return Module;
				}
			}
		}

		return nullptr;
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

		if (!serializer.Read(aFile, "ParticleEffect"))
		{ Log::error("Can't load Particle Effect: " + aFile); return false; }

		ParticleModuleAccelerationBase* tAccelerationBase = nullptr;
		ParticleModuleColorBase* tColorBase = nullptr;
		ParticleModuleEmitBase* tEmitBase = nullptr;
		ParticleModuleLifetimeBase* tLifetimeBase = nullptr;
		ParticleModuleLocationBase* tLocationBase = nullptr;
		ParticleModuleNoiseBase* tNoiseBase = nullptr;
		ParticleModuleRequiredBase* tRequiredBase = nullptr;
		ParticleModuleRotationBase* tRotationBase = nullptr;
		ParticleModuleSizeBase* tSizeBase = nullptr;
		ParticleModuleVelocityBase* tVelocityBase = nullptr;
		ParticleModuleSubUVBase* tSubUVBase = nullptr;

		ParticleModuleEmit* tEmit = new ParticleModuleEmit();

		if (serializer.GetSubBool({ "Emit", "Emitting" }, tEmit->Active) &&
		    serializer.GetSubInt({ "Emit", "Count" }, tEmit->Count) &&
		    serializer.GetSubFloat({ "Emit", "EmitRate" }, tEmit->EmitRate))
		{
			tEmitBase = tEmit;
		}

		ParticleModuleLocationCircle* tLocationCircle = new ParticleModuleLocationCircle();

		if (serializer.GetSubFloat({ "Location", "Circle", "Radius" }, tLocationCircle->Radius) &&
		    serializer.GetSubBool({ "Location", "Circle", "EmitFromShell" }, tLocationCircle->EmitFromShell))
		{
			tLocationBase = tLocationCircle;
		}

		ParticleModuleLocationBox* tLocationBox = new ParticleModuleLocationBox();

		if (serializer.GetSubVector3({ "Location", "Box", "Size" }, tLocationBox->Size, { "X", "Y", "Z" }) &&
		    serializer.GetSubBool({ "Location", "Box", "EmitFromShell" }, tLocationBox->EmitFromShell))
		{
			tLocationBase = tLocationBox;
		}

		ParticleModuleLocationSphere* tLocationSphere = new ParticleModuleLocationSphere();

		if (serializer.GetSubFloat({ "Location", "Sphere", "Radius" }, tLocationSphere->Radius) &&
		    serializer.GetSubBool({ "Location", "Sphere", "EmitFromShell" }, tLocationSphere->EmitFromShell))
		{
			tLocationBase = tLocationSphere;
		}

		ParticleModuleLifetime* tLifetime = new ParticleModuleLifetime();

		if (serializer.GetSubFloat({ "Lifetime", "Min" }, tLifetime->Min) &&
		    serializer.GetSubFloat({ "Lifetime", "Max" }, tLifetime->Max))
		{
			tLifetimeBase = tLifetime;
		}

		ParticleModuleRotation* tRotation = new ParticleModuleRotation();

		if (serializer.GetSubFloat({ "Rotation", "Min" }, tRotation->Min) &&
		    serializer.GetSubFloat({ "Rotation", "Max" }, tRotation->Max) &&
		    serializer.GetSubFloat({ "Rotation", "MinVelocity" }, tRotation->MinVelocity) &&
		    serializer.GetSubFloat({ "Rotation", "MaxVelocity" }, tRotation->MaxVelocity))
		{
			tRotationBase = tRotation;
		}

		ParticleModuleRequired* tRequired = new ParticleModuleRequired();

		if (serializer.GetSubBool({ "Required", "Visible" }, tRequired->Active) &&
		    serializer.GetSubBool({ "Required", "AdditiveBlending" }, tRequired->AdditiveBlending) &&
		    serializer.GetSubBool({ "Required", "Billboarding" }, tRequired->Billboarding) &&
		    serializer.GetSubInt({ "Required", "Transformation" }, (int32&)tRequired->Transformation) &&
		    serializer.GetSubInt({ "Required", "SortMode" }, (int32&)tRequired->SortMode))
		{
			tRequiredBase = tRequired;
		}

		ParticleModuleVelocity* tVelocity = new ParticleModuleVelocity();

		if (serializer.GetSubVector3({ "InitialVelocity", "Min" }, tVelocity->Min, { "X", "Y", "Z" }) &&
		    serializer.GetSubVector3({ "InitialVelocity", "Max" }, tVelocity->Max, { "X", "Y", "Z" }))
		{
			tVelocityBase = tVelocity;
		}

		ParticleModuleAcceleration* tAcceleration = new ParticleModuleAcceleration();

		if (serializer.GetSubVector3({ "Acceleration", "Initial", "Min" }, tAcceleration->Min, { "X", "Y", "Z" }) &&
		    serializer.GetSubVector3({ "Acceleration", "Initial", "Max" }, tAcceleration->Max, { "X", "Y", "Z" }))
		{
			tAccelerationBase = tAcceleration;
		}

		ParticleModuleSize* tSize = new ParticleModuleSize();

		if (serializer.GetSubVector3({ "Size", "Initial", "Min" }, tSize->Min, { "X", "Y", "Z" }) &&
		    serializer.GetSubVector3({ "Size", "Initial", "Max" }, tSize->Max, { "X", "Y", "Z" }))
		{
			tSizeBase = tSize;
		}

		ParticleModuleSizeOverLife* tSizeOverLife = new ParticleModuleSizeOverLife();

		if (serializer.GetSubVector3({ "Size", "OverLife", "MinStart" }, tSizeOverLife->MinStart, { "X", "Y", "Z" }) &&
		    serializer.GetSubVector3({ "Size", "OverLife", "MaxStart" }, tSizeOverLife->MaxStart, { "X", "Y", "Z" }) &&
		    serializer.GetSubVector3({ "Size", "OverLife", "MinFinal" }, tSizeOverLife->MinFinal, { "X", "Y", "Z" }) &&
		    serializer.GetSubVector3({ "Size", "OverLife", "MaxFinal" }, tSizeOverLife->MaxFinal, { "X", "Y", "Z" }))
		{
			tSizeBase = tSizeOverLife;
		}

		ParticleModuleColor* tColor = new ParticleModuleColor();

		if (serializer.GetSubVector4({ "Color", "Initial", "Min" }, tColor->Min, { "R", "G", "B", "A" }) &&
		    serializer.GetSubVector4({ "Color", "Initial", "Max" }, tColor->Max, { "R", "G", "B", "A" }))
		{
			tColorBase = tColor;
		}

		ParticleModuleColorOverLife* tColorOverLife = new ParticleModuleColorOverLife();

		if (serializer.GetSubVector4({ "Color", "OverLife", "MinStart" }, tColorOverLife->MinStart, { "R", "G", "B", "A" }) &&
		    serializer.GetSubVector4({ "Color", "OverLife", "MaxStart" }, tColorOverLife->MaxStart, { "R", "G", "B", "A" }) &&
		    serializer.GetSubVector4({ "Color", "OverLife", "MinFinal" }, tColorOverLife->MinFinal, { "R", "G", "B", "A" }) &&
		    serializer.GetSubVector4({ "Color", "OverLife", "MaxFinal" }, tColorOverLife->MaxFinal, { "R", "G", "B", "A" }))
		{
			tColorBase = tColorOverLife;
		}


		ParticleModuleNoise* tNoise = new ParticleModuleNoise();

		if (serializer.GetSubBool({ "Noise", "Active" }, tNoise->Active) &&
		    serializer.GetSubFloat({ "Noise", "Strength" }, tNoise->Strength) &&
		    serializer.GetSubInt({ "Noise", "Octaves" }, tNoise->Octaves) &&
		    serializer.GetSubFloat({ "Noise", "Lacunarity" }, tNoise->Lacunarity) &&
		    serializer.GetSubFloat({ "Noise", "Persistence" }, tNoise->Persistence) &&
		    serializer.GetSubFloat({ "Noise", "Frequency" }, tNoise->Frequency) &&
		    serializer.GetSubFloat({ "Noise", "Amplitude" }, tNoise->Amplitude))
		{
			tNoiseBase = tNoise;
		}

		ParticleModuleSubUV* tSubUV = new ParticleModuleSubUV();

		if (serializer.GetSubInt({ "SubUV", "Mode" }, (int32&)tSubUV->Mode) &&
		    serializer.GetSubInt({ "SubUV", "Horizontal" }, tSubUV->Horizontal) &&
		    serializer.GetSubInt({ "SubUV", "Vertical" }, tSubUV->Vertical) &&
		    serializer.GetSubFloat({ "SubUV", "Cycles" }, tSubUV->Cycles))
		{
			tSubUVBase = tSubUV;
		}

		Log::success("Particle Effect loaded: " + aFile);

		if (tAccelerationBase != nullptr)
		{
			AddModule(tAccelerationBase);
		}

		if (tColorBase != nullptr)
		{
			AddModule(tColorBase);
		}

		if (tEmitBase != nullptr)
		{
			AddModule(tEmitBase);
			Emit = static_cast<ParticleModuleEmit*>(tEmitBase);
		}

		if (tLifetimeBase != nullptr)
		{
			AddModule(tLifetimeBase);
		}

		if (tLocationBase != nullptr)
		{
			AddModule(tLocationBase);
		}

		if (tNoiseBase != nullptr)
		{
			AddModule(tNoiseBase);
		}

		if (tRequiredBase != nullptr)
		{
			AddModule(tRequiredBase);
			Required = static_cast<ParticleModuleRequired*>(tRequiredBase);
		}

		if (tRotationBase != nullptr)
		{
			AddModule(tRotationBase);
		}

		if (tSizeBase != nullptr)
		{
			AddModule(tSizeBase);
		}

		if (tVelocityBase != nullptr)
		{
			AddModule(tVelocityBase);
		}

		if (tSubUVBase != nullptr)
		{
			AddModule(tSubUVBase);	
		}

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
