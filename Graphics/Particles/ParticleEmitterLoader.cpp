#include <Graphics/Particles/ParticleEmitterLoader.h>
#include <Common/JSON/JSON.h>

namespace Columbus
{

	static void operator>>(JSON& J, ParticleEmitterCPU::BlendMode& Blend)
	{
		auto String = J["Blend"].GetString();
		if (String == "Default")  Blend = ParticleEmitterCPU::BlendMode::Default;
		if (String == "Add")      Blend = ParticleEmitterCPU::BlendMode::Add;
		if (String == "Subtract") Blend = ParticleEmitterCPU::BlendMode::Subtract;
		if (String == "Multiply") Blend = ParticleEmitterCPU::BlendMode::Multiply;
	}

	static void operator>>(JSON& J, ParticleEmitterCPU::BillboardMode& Billboard)
	{
		auto String = J["Billboard"].GetString();
		if (String == "None")         Billboard = ParticleEmitterCPU::BillboardMode::None;
		if (String == "Horizontal")   Billboard = ParticleEmitterCPU::BillboardMode::Horizontal;
		if (String == "Vertical")     Billboard = ParticleEmitterCPU::BillboardMode::Vertical;
		if (String == "FaceToCamera") Billboard = ParticleEmitterCPU::BillboardMode::FaceToCamera;
	}

	static void operator>>(JSON& J, ParticleEmitterCPU::TransformationMode& Transformation)
	{
		auto String = J["Transformation"].GetString();
		if (String == "World") Transformation = ParticleEmitterCPU::TransformationMode::World;
		if (String == "Local") Transformation = ParticleEmitterCPU::TransformationMode::Local;
	}

	static void operator>>(JSON& J, ParticleEmitterCPU::SortMode& Sort)
	{
		auto String = J["Sort"].GetString();
		if (String == "None")         Sort = ParticleEmitterCPU::SortMode::None;
		if (String == "YoungFirst")   Sort = ParticleEmitterCPU::SortMode::YoungFirst;
		if (String == "OldFirst")     Sort = ParticleEmitterCPU::SortMode::OldFirst;
		if (String == "NearestFirst") Sort = ParticleEmitterCPU::SortMode::NearestFirst;
	}

	static void operator>>(JSON& J, ParticleModuleLifetime& Lifetime)
	{
		Lifetime.Min = (float)J["Lifetime"]["Min"].GetFloat();
		Lifetime.Max = (float)J["Lifetime"]["Max"].GetFloat();
	}

	static void operator>>(JSON& J, ParticleModuleLocation& Location)
	{
		auto String = J["Location"]["Shape"].GetString();

		Location.Radius = (float)J["Location"]["Radius"].GetFloat();
		Location.Size = J["Location"]["Size"].GetVector3<float>();
		Location.EmitFromShell = J["Location"]["EmitFromShell"].GetBool();

		if (String == "Point")  Location.Shape = ParticleModuleLocation::SpawnShape::Point;
		if (String == "Box")    Location.Shape = ParticleModuleLocation::SpawnShape::Box;
		if (String == "Circle") Location.Shape = ParticleModuleLocation::SpawnShape::Circle;
		if (String == "Sphere") Location.Shape = ParticleModuleLocation::SpawnShape::Sphere;
	}

	static void operator>>(JSON& J, ParticleModuleVelocity& Velocity)
	{
		Velocity.Min = J["Velocity"]["Min"].GetVector3<float>();
		Velocity.Max = J["Velocity"]["Max"].GetVector3<float>();
	}

	static void operator>>(JSON& J, ParticleModuleRotation& Rotation)
	{
		Rotation.Min = (float)J["Rotation"]["Min"].GetFloat();
		Rotation.Max = (float)J["Rotation"]["Max"].GetFloat();
		Rotation.MinVelocity = (float)J["Rotation"]["MinVelocity"].GetFloat();
		Rotation.MaxVelocity = (float)J["Rotation"]["MaxVelocity"].GetFloat();
	}

	static void operator>>(JSON& J, ParticleModuleColor& Color)
	{
		auto String = J["Color"]["Mode"].GetString();

		Color.Min = J["Color"]["Min"].GetVector4<float>();
		Color.Max = J["Color"]["Max"].GetVector4<float>();
		
		for (size_t i = 0; i < J["Color"]["Curve"].GetElementsCount(); i++)
		{
			Color.Curve.AddPoint(J["Color"]["Curve"][i]["Color"].GetVector4<float>(), (float)J["Color"]["Curve"][i]["Key"].GetFloat());
		}

		if (String == "Initial")  Color.Mode = ParticleModuleColor::UpdateMode::Initial;
		if (String == "OverLife") Color.Mode = ParticleModuleColor::UpdateMode::OverLife;
	}

	static void operator>>(JSON& J, ParticleModuleSize& Size)
	{
		auto String = J["Size"]["Mode"].GetString();

		Size.Min = J["Size"]["Min"].GetVector3<float>();
		Size.Max = J["Size"]["Max"].GetVector3<float>();

		for (size_t i = 0; i < J["Size"]["Curve"].GetElementsCount(); i++)
		{
			Size.Curve.AddPoint(J["Size"]["Curve"][i]["Size"].GetVector3<float>(), (float)J["Size"]["Curve"][i]["Key"].GetFloat());
		}

		if (String == "Initial")  Size.Mode = ParticleModuleSize::UpdateMode::Initial;
		if (String == "OverLife") Size.Mode = ParticleModuleSize::UpdateMode::OverLife;
	}

	static void operator>>(JSON& J, ParticleModuleSubUV& SubUV)
	{
		auto String = J["SubUV"]["Mode"].GetString();

		SubUV.Horizontal = (int)J["SubUV"]["Horizontal"].GetInt();
		SubUV.Vertical = (int)J["SubUV"]["Vertical"].GetInt();
		SubUV.Cycles = (float)J["SubUV"]["Cycles"].GetFloat();

		if (String == "Linear") SubUV.Mode = ParticleModuleSubUV::SubUVMode::Linear;
		if (String == "Random") SubUV.Mode = ParticleModuleSubUV::SubUVMode::Random;
	}

	bool ParticleEmitterLoader::Load(ParticleEmitterCPU& Particles, const char* FileName)
	{
		JSON J;
		if (!J.Load(FileName)) return false;

		Particles.Visible = J["Visible"].GetBool();
		Particles.Emit = J["Emit"].GetBool();
		Particles.MaxParticles = (uint32)J["MaxCount"].GetInt();
		Particles.EmitRate = (float)J["EmitRate"].GetFloat();
		J >> Particles.Blend;
		J >> Particles.Billboard;
		J >> Particles.Transformation;
		J >> Particles.Sort;
		
		J >> Particles.ModuleLifetime;
		J >> Particles.ModuleLocation;
		J >> Particles.ModuleVelocity;
		J >> Particles.ModuleRotation;
		J >> Particles.ModuleColor;
		J >> Particles.ModuleSize;
		//TODO: Noise
		J >> Particles.ModuleSubUV;

		return true;
	}

}


