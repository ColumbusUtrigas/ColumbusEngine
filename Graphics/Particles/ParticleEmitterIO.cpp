#include <Graphics/Particles/ParticleEmitterCPU.h>
#include <Common/JSON/JSON.h>

namespace Columbus
{

	JSON_SERIALIZE_ENUM(ParticleEmitterCPU::BlendMode, {
		{ ParticleEmitterCPU::BlendMode::Default, "Default" },
		{ ParticleEmitterCPU::BlendMode::Add, "Add" },
		{ ParticleEmitterCPU::BlendMode::Subtract, "Subtract" },
		{ ParticleEmitterCPU::BlendMode::Multiply, "Multiply" }
		});

	JSON_SERIALIZE_ENUM(ParticleEmitterCPU::BillboardMode, {
		{ ParticleEmitterCPU::BillboardMode::None, "None" },
		{ ParticleEmitterCPU::BillboardMode::Horizontal, "Horizontal" },
		{ ParticleEmitterCPU::BillboardMode::Vertical, "Vertical" },
		{ ParticleEmitterCPU::BillboardMode::FaceToCamera, "FaceToCamera" }
		});

	JSON_SERIALIZE_ENUM(ParticleEmitterCPU::TransformationMode, {
		{ ParticleEmitterCPU::TransformationMode::World, "World" },
		{ ParticleEmitterCPU::TransformationMode::Local, "Local" }
		});

	JSON_SERIALIZE_ENUM(ParticleEmitterCPU::SortMode, {
		{ ParticleEmitterCPU::SortMode::None, "None" },
		{ ParticleEmitterCPU::SortMode::YoungFirst, "YoungFirst" },
		{ ParticleEmitterCPU::SortMode::OldFirst, "OldFirst" },
		{ ParticleEmitterCPU::SortMode::NearestFirst, "NearestFirst" }
		});

	JSON_SERIALIZE_ENUM(ParticleModuleLocation::SpawnShape, {
		{ ParticleModuleLocation::SpawnShape::Point, "Point" },
		{ ParticleModuleLocation::SpawnShape::Box, "Box" },
		{ ParticleModuleLocation::SpawnShape::Circle, "Circle" },
		{ ParticleModuleLocation::SpawnShape::Sphere, "Sphere" }
		});

	JSON_SERIALIZE_ENUM(ParticleModuleColor::UpdateMode, {
		{ ParticleModuleColor::UpdateMode::Initial, "Initial" },
		{ ParticleModuleColor::UpdateMode::OverLife, "OverLife" }
		});

	JSON_SERIALIZE_ENUM(ParticleModuleSize::UpdateMode, {
		{ ParticleModuleSize::UpdateMode::Initial, "Initial" },
		{ ParticleModuleSize::UpdateMode::OverLife, "OverLife" }
		});

	JSON_SERIALIZE_ENUM(ParticleModuleSubUV::SubUVMode, {
		{ ParticleModuleSubUV::SubUVMode::Linear, "Linear" },
		{ ParticleModuleSubUV::SubUVMode::Random, "Random" }
		});

	static void operator>>(JSON& J, ParticleModuleLifetime& Lifetime)
	{
		Lifetime.Min = (float)J["Min"];
		Lifetime.Max = (float)J["Max"];
	}

	static void operator<<(JSON& J, const ParticleModuleLifetime& Lifetime)
	{
		J["Min"] = Lifetime.Min;
		J["Max"] = Lifetime.Max;
	}

	static void operator>>(JSON& J, ParticleModuleLocation& Location)
	{
		Location.Radius = (float)J["Radius"];
		Location.Size = J["Size"];
		Location.EmitFromShell = (bool)J["EmitFromShell"];
		J["Shape"] >> Location.Shape;
	}

	static void operator<<(JSON& J, const ParticleModuleLocation& Location)
	{
		J["Radius"] = Location.Radius;
		J["Size"] = Location.Size;
		J["EmitFromShell"] = Location.EmitFromShell;
		J["Shape"] << Location.Shape;
	}

	static void operator>>(JSON& J, ParticleModuleVelocity& Velocity)
	{
		Velocity.Min = J["Min"];
		Velocity.Max = J["Max"];
	}

	static void operator<<(JSON& J, const ParticleModuleVelocity& Velocity)
	{
		J["Min"] = Velocity.Min;
		J["Max"] = Velocity.Max;
	}

	static void operator>>(JSON& J, ParticleModuleRotation& Rotation)
	{
		Rotation.Min = (float)J["Min"];
		Rotation.Max = (float)J["Max"];
		Rotation.MinVelocity = (float)J["MinVelocity"];
		Rotation.MaxVelocity = (float)J["MaxVelocity"];
	}

	static void operator<<(JSON& J, const ParticleModuleRotation& Rotation)
	{
		J["Min"] = Rotation.Min;
		J["Max"] = Rotation.Max;
		J["MinVelocity"] = Rotation.MinVelocity;
		J["MaxVelocity"] = Rotation.MaxVelocity;
	}

	static void operator>>(JSON& J, ParticleModuleColor& Color)
	{
		Color.Min = J["Min"];
		Color.Max = J["Max"];
		J["Mode"] >> Color.Mode;
		
		for (size_t i = 0; i < J["Curve"].GetElementsCount(); i++)
		{
			Color.Curve.AddPoint(J["Curve"][i]["Color"], (float)J["Curve"][i]["Key"]);
		}
	}

	static void operator<<(JSON& J, const ParticleModuleColor& Color)
	{
		J["Min"] = Color.Min;
		J["Max"] = Color.Max;
		J["Mode"] << Color.Mode;

		for (uint32 i = 0; i < Color.Curve.Points.size(); i++)
		{
			J["Curve"][i]["Key"] = Color.Curve.Points[i].Key;
			J["Curve"][i]["Color"] = Color.Curve.Points[i].Value;
		}
	}

	static void operator>>(JSON& J, ParticleModuleSize& Size)
	{
		Size.Min = J["Min"];
		Size.Max = J["Max"];
		J["Mode"] >> Size.Mode;

		for (size_t i = 0; i < J["Curve"].GetElementsCount(); i++)
		{
			Size.Curve.AddPoint(J["Curve"][i]["Size"], (float)J["Curve"][i]["Key"]);
		}
	}

	static void operator<<(JSON& J, const ParticleModuleSize& Size)
	{
		J["Min"] = Size.Min;
		J["Max"] = Size.Max;
		J["Mode"] << Size.Mode;

		for (uint32 i = 0; i < Size.Curve.Points.size(); i++)
		{
			J["Curve"][i]["Key"] = Size.Curve.Points[i].Key;
			J["Curve"][i]["Size"] = Size.Curve.Points[i].Value;
		}
	}

	static void operator>>(JSON& J, ParticleModuleSubUV& SubUV)
	{
		SubUV.Horizontal = (int)J["Horizontal"];
		SubUV.Vertical = (int)J["Vertical"];
		SubUV.Cycles = (float)J["Cycles"];
		J["Mode"] >> SubUV.Mode;
	}

	static void operator<<(JSON& J, const ParticleModuleSubUV& SubUV)
	{
		J["Horizontal"] = SubUV.Horizontal;
		J["Vertical"] = SubUV.Vertical;
		J["Cycles"] = SubUV.Cycles;
		J["Mode"] << SubUV.Mode;
	}

	void ParticleEmitterCPU::Serialize(JSON& J) const
	{
		J["Visible"] = Visible;
		J["Emit"] = Emit;
		J["Light"] = Light;
		J["MaxCount"] = MaxParticles;
		J["EmitRate"] = EmitRate;
		J["Blend"] << Blend;
		J["Billboard"] << Billboard;
		J["Transformation"] << Transformation;
		J["Sort"] << Sort;
		
		J["Lifetime"] << ModuleLifetime;
		J["Location"] << ModuleLocation;
		J["Velocity"] << ModuleVelocity;
		J["Rotation"] << ModuleRotation;
		J["Color"] << ModuleColor;
		J["Size"] << ModuleSize;
		//TODO: Noise
		J["SubUV"] << ModuleSubUV;
	}

	void ParticleEmitterCPU::Deserialize(JSON& J)
	{
		Visible = (bool)J["Visible"];
		Emit = (bool)J["Emit"];
		Light = (bool)J["Light"];
		MaxParticles = (uint32)J["MaxCount"];
		EmitRate = (float)J["EmitRate"];

		J["Blend"] >> Blend;
		J["Billboard"] >> Billboard;
		J["Transformation"] >> Transformation;
		J["Sort"] >> Sort;
		
		J["Lifetime"] >> ModuleLifetime;
		J["Location"] >> ModuleLocation;
		J["Velocity"] >> ModuleVelocity;
		J["Rotation"] >> ModuleRotation;
		J["Color"] >> ModuleColor;
		J["Size"] >> ModuleSize;
		//TODO: Noise
		J["SubUV"] >> ModuleSubUV;
	}

}
