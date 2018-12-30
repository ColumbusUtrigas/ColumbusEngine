#include <Graphics/Particles/ParticleEffect.h>

#include <Graphics/Particles/Acceleration/ParticleModuleAcceleration.h>
#include <Graphics/Particles/Color/ParticleModuleColor.h>
#include <Graphics/Particles/Color/ParticleModuleColorOverLife.h>
#include <Graphics/Particles/Emit/ParticleModuleEmit.h>
#include <Graphics/Particles/Location/ParticleModuleLocationBase.h>
#include <Graphics/Particles/Location/ParticleModuleLocationBox.h>
#include <Graphics/Particles/Location/ParticleModuleLocationCircle.h>
#include <Graphics/Particles/Location/ParticleModuleLocationSphere.h>
#include <Graphics/Particles/Lifetime/ParticleModuleLifetime.h>
#include <Graphics/Particles/Noise/ParticleModuleNoise.h>
#include <Graphics/Particles/Required/ParticleModuleRequired.h>
#include <Graphics/Particles/Rotation/ParticleModuleRotation.h>
#include <Graphics/Particles/Size/ParticleModuleSize.h>
#include <Graphics/Particles/Size/ParticleModuleSizeOverLife.h>
#include <Graphics/Particles/Velocity/ParticleModuleVelocity.h>
#include <Graphics/Particles/SubUV/ParticleModuleSubUV.h>

#include <System/Serializer.h>

namespace Columbus
{

	ParticleEffect::ParticleEffect()
	{

	}
	
	ParticleEffect::ParticleEffect(std::string aFile)
	{
		load(aFile);
	}
	
	ParticleEffect::ParticleEffect(std::string aFile, Material* aMaterial)
	{
		load(aFile);
		mMaterial = aMaterial;
	}
	
	void ParticleEffect::AddModule(ParticleModule* Module)
	{
		if (Module == nullptr) return;
		if (GetModule(Module->GetType()) != nullptr) return;
		Modules.push_back(Module);
	}
	
	ParticleModule* ParticleEffect::GetModule(ParticleModule::Type Type) const
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
	
	void ParticleEffect::setMaterial(const Material* aMaterial)
	{
		mMaterial = const_cast<Material*>(aMaterial);
	}
	
	void ParticleEffect::setPos(const Vector3 aPos)
	{
		mPos = static_cast<Vector3>(aPos);
	}
	
	void ParticleEffect::addPos(const Vector3 aPos)
	{
		mPos += static_cast<Vector3>(aPos);
	}
	
	Material* ParticleEffect::getMaterial() const
	{
		return mMaterial;
	}
	
	Vector3 ParticleEffect::getPos() const
	{
		return mPos;
	}
	
	bool ParticleEffect::saveToXML(std::string aFile) const
	{

		return true;
	}
	
	bool ParticleEffect::loadFromXML(std::string aFile)
	{
		Serializer::SerializerXML serializer;

		if (!serializer.Read(aFile, "ParticleEffect"))
		{ Log::error("Can't load Particle Effect: " + aFile); return false; }

		ParticleModule* tAccelerationBase = nullptr;
		ParticleModule* tColorBase = nullptr;
		ParticleModule* tEmitBase = nullptr;
		ParticleModule* tLifetimeBase = nullptr;
		ParticleModuleLocationBase* tLocationBase = nullptr;
		ParticleModule* tNoiseBase = nullptr;
		ParticleModule* tRequiredBase = nullptr;
		ParticleModule* tRotationBase = nullptr;
		ParticleModule* tSizeBase = nullptr;
		ParticleModule* tVelocityBase = nullptr;
		ParticleModule* tSubUVBase = nullptr;

		ParticleModuleEmit* tEmit = new ParticleModuleEmit();

		if (serializer.GetSubBool({ "Emit", "Emitting" }, tEmit->Active) &&
		    serializer.GetSubInt({ "Emit", "Count" }, tEmit->Count) &&
		    serializer.GetSubFloat({ "Emit", "EmitRate" }, tEmit->EmitRate))
		{
			tEmitBase = tEmit;
		} else delete tEmit;

		ParticleModuleLocationCircle* tLocationCircle = new ParticleModuleLocationCircle();

		if (serializer.GetSubFloat({ "Location", "Circle", "Radius" }, tLocationCircle->Radius) &&
		    serializer.GetSubBool({ "Location", "Circle", "EmitFromShell" }, tLocationCircle->EmitFromShell))
		{
			tLocationBase = tLocationCircle;
		} else delete tLocationCircle;

		ParticleModuleLocationBox* tLocationBox = new ParticleModuleLocationBox();

		if (serializer.GetSubVector3({ "Location", "Box", "Size" }, tLocationBox->Size, { "X", "Y", "Z" }) &&
		    serializer.GetSubBool({ "Location", "Box", "EmitFromShell" }, tLocationBox->EmitFromShell))
		{
			tLocationBase = tLocationBox;
		} else delete tLocationBox;

		ParticleModuleLocationSphere* tLocationSphere = new ParticleModuleLocationSphere();

		if (serializer.GetSubFloat({ "Location", "Sphere", "Radius" }, tLocationSphere->Radius) &&
		    serializer.GetSubBool({ "Location", "Sphere", "EmitFromShell" }, tLocationSphere->EmitFromShell))
		{
			tLocationBase = tLocationSphere;
		} else delete tLocationSphere;

		ParticleModuleLifetime* tLifetime = new ParticleModuleLifetime();

		if (serializer.GetSubFloat({ "Lifetime", "Min" }, tLifetime->Min) &&
		    serializer.GetSubFloat({ "Lifetime", "Max" }, tLifetime->Max))
		{
			tLifetimeBase = tLifetime;
		} else delete tLifetime;

		ParticleModuleRotation* tRotation = new ParticleModuleRotation();

		if (serializer.GetSubFloat({ "Rotation", "Min" }, tRotation->Min) &&
		    serializer.GetSubFloat({ "Rotation", "Max" }, tRotation->Max) &&
		    serializer.GetSubFloat({ "Rotation", "MinVelocity" }, tRotation->MinVelocity) &&
		    serializer.GetSubFloat({ "Rotation", "MaxVelocity" }, tRotation->MaxVelocity))
		{
			tRotationBase = tRotation;
		} else delete tRotation;

		ParticleModuleRequired* tRequired = new ParticleModuleRequired();

		if (serializer.GetSubBool({ "Required", "Visible" }, tRequired->Active) &&
		    serializer.GetSubBool({ "Required", "AdditiveBlending" }, tRequired->AdditiveBlending) &&
		    serializer.GetSubBool({ "Required", "Billboarding" }, tRequired->Billboarding) &&
		    serializer.GetSubInt({ "Required", "Transformation" }, (int32&)tRequired->Transformation) &&
		    serializer.GetSubInt({ "Required", "SortMode" }, (int32&)tRequired->SortMode))
		{
			tRequiredBase = tRequired;
		} else delete tRequired;

		ParticleModuleVelocity* tVelocity = new ParticleModuleVelocity();

		if (serializer.GetSubVector3({ "InitialVelocity", "Min" }, tVelocity->Min, { "X", "Y", "Z" }) &&
		    serializer.GetSubVector3({ "InitialVelocity", "Max" }, tVelocity->Max, { "X", "Y", "Z" }))
		{
			tVelocityBase = tVelocity;
		} else delete tVelocity;

		ParticleModuleAcceleration* tAcceleration = new ParticleModuleAcceleration();

		if (serializer.GetSubVector3({ "Acceleration", "Initial", "Min" }, tAcceleration->Min, { "X", "Y", "Z" }) &&
		    serializer.GetSubVector3({ "Acceleration", "Initial", "Max" }, tAcceleration->Max, { "X", "Y", "Z" }))
		{
			tAccelerationBase = tAcceleration;
		} else delete tAcceleration;

		ParticleModuleSize* tSize = new ParticleModuleSize();

		if (serializer.GetSubVector3({ "Size", "Initial", "Min" }, tSize->Min, { "X", "Y", "Z" }) &&
		    serializer.GetSubVector3({ "Size", "Initial", "Max" }, tSize->Max, { "X", "Y", "Z" }))
		{
			tSizeBase = tSize;
		} else delete tSize;

		ParticleModuleSizeOverLife* tSizeOverLife = new ParticleModuleSizeOverLife();
		auto SizeOverLifeElement = serializer.GetSubElement({ "Size", "OverLife", "SizeKey" });

		if (SizeOverLifeElement != nullptr)
		{
			Vector3 Value;
			float Key;

			while (SizeOverLifeElement != nullptr)
			{
				if (serializer.GetVector3(SizeOverLifeElement, Value, { "X", "Y", "Z" }) &&
				    serializer.GetFloat(SizeOverLifeElement, Key))
				{
					tSizeOverLife->SizeCurve.AddPoint(Value, Key);
				}

				SizeOverLifeElement = serializer.NextElement(SizeOverLifeElement, "SizeKey");
			}

			tSizeBase = tSizeOverLife;
		} else delete tSizeOverLife;

		delete SizeOverLifeElement;

		ParticleModuleColor* tColor = new ParticleModuleColor();

		if (serializer.GetSubVector4({ "Color", "Initial", "Min" }, tColor->Min, { "R", "G", "B", "A" }) &&
		    serializer.GetSubVector4({ "Color", "Initial", "Max" }, tColor->Max, { "R", "G", "B", "A" }))
		{
			tColorBase = tColor;
		} else delete tColor;

		ParticleModuleColorOverLife* tColorOverLife = new ParticleModuleColorOverLife();
		auto Elem = serializer.GetSubElement({ "Color", "OverLife", "ColorKey" });

		if (Elem != nullptr)
		{
			Vector4 Value;
			float Key;

			while (Elem != nullptr)
			{
				if (serializer.GetVector4(Elem, Value, { "R", "G", "B", "A" }) &&
				    serializer.GetFloat(Elem, Key))
				{
					tColorOverLife->ColorCurve.AddPoint(Value, Key);
				}

				Elem = serializer.NextElement(Elem, "ColorKey");
			}

			tColorBase = tColorOverLife;
		} else delete tColorOverLife;

		delete Elem;

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
		} else delete tNoise;

		ParticleModuleSubUV* tSubUV = new ParticleModuleSubUV();

		if (serializer.GetSubInt({ "SubUV", "Mode" }, (int32&)tSubUV->Mode) &&
		    serializer.GetSubInt({ "SubUV", "Horizontal" }, tSubUV->Horizontal) &&
		    serializer.GetSubInt({ "SubUV", "Vertical" }, tSubUV->Vertical) &&
		    serializer.GetSubFloat({ "SubUV", "Cycles" }, tSubUV->Cycles))
		{
			tSubUVBase = tSubUV;
		} else delete tSubUV;

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
	
	bool ParticleEffect::load(std::string aFile)
	{
		if (aFile.find_last_of(".cxpar") != std::string::npos)
		{
			return loadFromXML(aFile);
		}
		
		return false;
	}
	
	ParticleEffect::~ParticleEffect()
	{

	}

}
