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

#include <Common/JSON/JSON.h>
#include <System/Log.h>

namespace Columbus
{

	ParticleEffect::ParticleEffect() {}
	
	void ParticleEffect::AddModule(ParticleModule* Module)
	{
		if (Module == nullptr) return;
		if (GetModule(Module->GetType()) != nullptr) return;
		Modules.Emplace(Module);
	}
	
	ParticleModule* ParticleEffect::GetModule(ParticleModule::Type Type) const
	{
		for (auto& Module : Modules)
		{
			if (Module.Get()->GetType() == Type)
			{
				return Module.Get();
			}
		}

		return nullptr;
	}

	bool ParticleEffect::Load(const char* FileName)
	{
		JSON J;
		if (!J.Load(FileName)) return false;

		ParticleModuleLocationBase* LocationBase = nullptr;
		ParticleModuleLifetime* Lifetime = nullptr;
		ParticleModuleRotation* Rotation = nullptr;
		ParticleModuleVelocity* Velocity = nullptr;
		ParticleModuleAcceleration* Acceleration = nullptr;
		ParticleModule* SizeBase = nullptr;
		ParticleModule* ColorBase = nullptr;
		ParticleModuleNoise* Noise = nullptr;
		ParticleModuleSubUV* SubUV = nullptr;

		Required.Visible = J["Required"]["Visible"].GetBool();
		Required.AdditiveBlending = J["Required"]["AdditiveBlending"].GetBool();
		Required.Billboarding = J["Required"]["Billboarding"].GetBool();
		if (J["Required"]["Transformation"].GetString() == "Local") Required.Transformation = ParticleTransformation::Local;
		if (J["Required"]["Transformation"].GetString() == "World") Required.Transformation = ParticleTransformation::World;
		if (J["Required"]["SortMode"].GetString() == "None")       Required.SortMode = ParticleSortMode::None;
		if (J["Required"]["SortMode"].GetString() == "Distance")   Required.SortMode = ParticleSortMode::Distance;
		if (J["Required"]["SortMode"].GetString() == "YoungFirst") Required.SortMode = ParticleSortMode::YoungFirst;
		if (J["Required"]["SortMode"].GetString() == "OldFirst")   Required.SortMode = ParticleSortMode::OldFirst;

		Emit.Active = J["Emit"]["Active"].GetBool();
		Emit.Count = (int)J["Emit"]["Count"].GetInt();
		Emit.EmitRate = (float)J["Emit"]["Rate"].GetFloat();

		if (J["LocationBox"].IsObject())
		{
			delete LocationBase;
			ParticleModuleLocationBox* Box = new ParticleModuleLocationBox();
			Box->Size = J["LocationBox"]["Size"].GetVector3<float>();
			Box->EmitFromShell = J["LocationBox"]["EmitFromShell"].GetBool();
			LocationBase = Box;
		}

		if (J["LocationCircle"].IsObject())
		{
			delete LocationBase;
			ParticleModuleLocationCircle* Circle = new ParticleModuleLocationCircle();
			Circle->Radius = (float)J["LocationCircle"]["Radius"].GetFloat();
			Circle->EmitFromShell = J["LocationCircle"]["EmitFromShell"].GetBool();
			LocationBase = Circle;
		}

		if (J["LocationSphere"].IsObject())
		{
			delete LocationBase;
			ParticleModuleLocationSphere* Sphere = new ParticleModuleLocationSphere();
			Sphere->Radius = (float)J["LocationSphere"]["Radius"].GetFloat();
			Sphere->EmitFromShell = J["LocationSphere"]["EmitFromShell"].GetBool();
			LocationBase = Sphere;
		}

		if (J["Lifetime"].IsObject())
		{
			Lifetime = new ParticleModuleLifetime();
			Lifetime->Min = (float)J["Lifetime"]["Min"].GetFloat();
			Lifetime->Max = (float)J["Lifetime"]["Max"].GetFloat();
		}

		if (J["Rotation"].IsObject())
		{
			Rotation = new ParticleModuleRotation();
			Rotation->Min = (float)J["Rotation"]["Min"].GetFloat();
			Rotation->Max = (float)J["Rotation"]["Max"].GetFloat();
			Rotation->MinVelocity = (float)J["Rotation"]["MinVelocity"].GetFloat();
			Rotation->MaxVelocity = (float)J["Rotation"]["MaxVelocity"].GetFloat();
		}

		if (J["InitialVelocity"].IsObject())
		{
			Velocity = new ParticleModuleVelocity();
			Velocity->Min = J["InitialVelocity"]["Min"].GetVector3<float>();
			Velocity->Max = J["InitialVelocity"]["Max"].GetVector3<float>();
		}

		if (J["InitialAcceleration"].IsObject())
		{
			Acceleration = new ParticleModuleAcceleration();
			Acceleration->Min = J["InitialAcceleration"]["Min"].GetVector3<float>();
			Acceleration->Max = J["InitialAcceleration"]["Max"].GetVector3<float>();
		}

		if (J["InitialSize"].IsObject())
		{
			delete SizeBase;
			ParticleModuleSize* Size = new ParticleModuleSize();
			Size->Min = J["InitialSize"]["Min"].GetVector3<float>();
			Size->Max = J["InitialSize"]["Max"].GetVector3<float>();
			SizeBase = Size;
		}

		if (J["SizeOverLifetime"].IsArray())
		{
			delete SizeBase;
			ParticleModuleSizeOverLife* SizeOverLife = new ParticleModuleSizeOverLife();
			
			for (uint32 i = 0; i < J["SizeOverLifetime"].GetElementsCount(); i++)
			{
				float Position = (float)J["SizeOverLifetime"][i]["Key"].GetFloat();
				Vector3 Point = J["SizeOverLifetime"][i]["Size"].GetVector3<float>();
				SizeOverLife->SizeCurve.AddPoint(Point, Position);
			}

			SizeBase = SizeOverLife;
		}

		if (J["InitialColor"].IsObject())
		{
			delete ColorBase;
			ParticleModuleColor* Color = new ParticleModuleColor();
			Color->Min = J["InitialColor"]["Min"].GetVector4<float>();
			Color->Max = J["InitialColor"]["Max"].GetVector4<float>();
			ColorBase = Color;
		}

		if (J["ColorOverLifetime"].IsArray())
		{
			delete ColorBase;
			ParticleModuleColorOverLife* ColorOverLife = new ParticleModuleColorOverLife();

			for (uint32 i = 0; i < J["ColorOverLifetime"].GetElementsCount(); i++)
			{
				float Position = (float)J["ColorOverLifetime"][i]["Key"].GetFloat();
				Vector4 Point = J["ColorOverLifetime"][i]["Color"].GetVector4<float>();
				ColorOverLife->ColorCurve.AddPoint(Point, Position);
			}

			ColorBase = ColorOverLife;
		}

		if (J["Noise"].IsObject())
		{
			Noise = new ParticleModuleNoise();
			Noise->Active      = J["Noise"]["Active"].GetBool();
			Noise->Strength    = (float)J["Noise"]["Strength"].GetFloat();
			Noise->Octaves     = (int)J["Noise"]["Octaves"].GetInt();
			Noise->Lacunarity  = (float)J["Noise"]["Lacunarity"].GetFloat();
			Noise->Persistence = (float)J["Noise"]["Persistence"].GetFloat();
			Noise->Frequency   = (float)J["Noise"]["Frequency"].GetFloat();
			Noise->Amplitude   = (float)J["Noise"]["Amplitude"].GetFloat();
		}

		if (J["SubUV"].IsObject())
		{
			SubUV = new ParticleModuleSubUV();
			if (J["SubUV"]["Mode"].GetString() == "Linear") SubUV->Mode = ParticleModuleSubUV::SubUVMode::Linear;
			if (J["SubUV"]["Mode"].GetString() == "Random") SubUV->Mode = ParticleModuleSubUV::SubUVMode::Random;
			SubUV->Horizontal = (int)J["SubUV"]["Horizontal"].GetInt();
			SubUV->Vertical = (int)J["SubUV"]["Vertical"].GetInt();
			SubUV->Cycles = (float)J["SubUV"]["Cycles"].GetFloat();
		}

		if (LocationBase != nullptr) AddModule(LocationBase);
		if (Lifetime != nullptr) AddModule(Lifetime);
		if (Rotation != nullptr) AddModule(Rotation);
		if (Velocity != nullptr) AddModule(Velocity);
		if (Acceleration != nullptr) AddModule(Acceleration);
		if (SizeBase != nullptr) AddModule(SizeBase);
		if (ColorBase != nullptr) AddModule(ColorBase);
		if (Noise != nullptr) AddModule(Noise);
		if (SubUV != nullptr) AddModule(SubUV);

		Log::Success("Particle Effect loaded: %s", FileName);

		return true;
	}
	
	ParticleEffect::~ParticleEffect() {}

}


