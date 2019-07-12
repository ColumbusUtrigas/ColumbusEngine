#include <Graphics/Particles/ParticleEmitterCPU.h>
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

	static void operator<<(JSON& J, const ParticleEmitterCPU::BlendMode& Blend)
	{
		switch (Blend)
		{
		case ParticleEmitterCPU::BlendMode::Default: J["Blend"] = "Default"; break;
		case ParticleEmitterCPU::BlendMode::Add: J["Blend"] = "Add"; break;
		case ParticleEmitterCPU::BlendMode::Subtract: J["Blend"] = "Subtract"; break;
		case ParticleEmitterCPU::BlendMode::Multiply: J["Blend"] = "Multiply"; break;
		}
	}

	static void operator>>(JSON& J, ParticleEmitterCPU::BillboardMode& Billboard)
	{
		auto String = J["Billboard"].GetString();
		if (String == "None")         Billboard = ParticleEmitterCPU::BillboardMode::None;
		if (String == "Horizontal")   Billboard = ParticleEmitterCPU::BillboardMode::Horizontal;
		if (String == "Vertical")     Billboard = ParticleEmitterCPU::BillboardMode::Vertical;
		if (String == "FaceToCamera") Billboard = ParticleEmitterCPU::BillboardMode::FaceToCamera;
	}

	static void operator<<(JSON& J, const ParticleEmitterCPU::BillboardMode& Billboard)
	{
		switch (Billboard)
		{
		case ParticleEmitterCPU::BillboardMode::None: J["Billboard"] = "None"; break;
		case ParticleEmitterCPU::BillboardMode::Horizontal: J["Billboard"] = "Horizontal"; break;
		case ParticleEmitterCPU::BillboardMode::Vertical: J["Billboard"] = "Vertical"; break;
		case ParticleEmitterCPU::BillboardMode::FaceToCamera: J["Billboard"] = "FaceToCamera"; break;
		}
	}

	static void operator>>(JSON& J, ParticleEmitterCPU::TransformationMode& Transformation)
	{
		auto String = J["Transformation"].GetString();
		if (String == "World") Transformation = ParticleEmitterCPU::TransformationMode::World;
		if (String == "Local") Transformation = ParticleEmitterCPU::TransformationMode::Local;
	}

	static void operator<<(JSON& J, const ParticleEmitterCPU::TransformationMode& Transformation)
	{
		switch (Transformation)
		{
		case ParticleEmitterCPU::TransformationMode::World: J["Transformation"] = "World"; break;
		case ParticleEmitterCPU::TransformationMode::Local: J["Transformation"] = "Local"; break;
		}
	}

	static void operator>>(JSON& J, ParticleEmitterCPU::SortMode& Sort)
	{
		auto String = J["Sort"].GetString();
		if (String == "None")         Sort = ParticleEmitterCPU::SortMode::None;
		if (String == "YoungFirst")   Sort = ParticleEmitterCPU::SortMode::YoungFirst;
		if (String == "OldFirst")     Sort = ParticleEmitterCPU::SortMode::OldFirst;
		if (String == "NearestFirst") Sort = ParticleEmitterCPU::SortMode::NearestFirst;
	}

	static void operator<<(JSON& J, const ParticleEmitterCPU::SortMode& Sort)
	{
		switch (Sort)
		{
		case ParticleEmitterCPU::SortMode::None: J["Sort"] = "None"; break;
		case ParticleEmitterCPU::SortMode::YoungFirst: J["Sort"] = "YoungFirst"; break;
		case ParticleEmitterCPU::SortMode::OldFirst: J["Sort"] = "OldFirst"; break;
		case ParticleEmitterCPU::SortMode::NearestFirst: J["Sort"] = "NearestFirst"; break;
		}
	}

	static void operator>>(JSON& J, ParticleModuleLifetime& Lifetime)
	{
		Lifetime.Min = (float)J["Lifetime"]["Min"].GetFloat();
		Lifetime.Max = (float)J["Lifetime"]["Max"].GetFloat();
	}

	static void operator<<(JSON& J, const ParticleModuleLifetime& Lifetime)
	{
		J["Lifetime"]["Min"] = Lifetime.Min;
		J["Lifetime"]["Max"] = Lifetime.Max;
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

	static void operator<<(JSON& J, const ParticleModuleLocation& Location)
	{
		J["Location"]["Radius"] = Location.Radius;
		J["Location"]["Size"] = Location.Size;
		J["Location"]["EmitFromShell"] = Location.EmitFromShell;

		switch (Location.Shape)
		{
		case ParticleModuleLocation::SpawnShape::Point: J["Location"]["Shape"] = "Point"; break;
		case ParticleModuleLocation::SpawnShape::Box: J["Location"]["Shape"] = "Box"; break;
		case ParticleModuleLocation::SpawnShape::Circle: J["Location"]["Shape"] = "Circle"; break;
		case ParticleModuleLocation::SpawnShape::Sphere: J["Location"]["Shape"] = "Sphere"; break;
		}
	}

	static void operator>>(JSON& J, ParticleModuleVelocity& Velocity)
	{
		Velocity.Min = J["Velocity"]["Min"].GetVector3<float>();
		Velocity.Max = J["Velocity"]["Max"].GetVector3<float>();
	}

	static void operator<<(JSON& J, const ParticleModuleVelocity& Velocity)
	{
		J["Velocity"]["Min"] = Velocity.Min;
		J["Velocity"]["Max"] = Velocity.Max;
	}

	static void operator>>(JSON& J, ParticleModuleRotation& Rotation)
	{
		Rotation.Min = (float)J["Rotation"]["Min"].GetFloat();
		Rotation.Max = (float)J["Rotation"]["Max"].GetFloat();
		Rotation.MinVelocity = (float)J["Rotation"]["MinVelocity"].GetFloat();
		Rotation.MaxVelocity = (float)J["Rotation"]["MaxVelocity"].GetFloat();
	}

	static void operator<<(JSON& J, const ParticleModuleRotation& Rotation)
	{
		J["Rotation"]["Min"] = Rotation.Min;
		J["Rotation"]["Max"] = Rotation.Max;
		J["Rotation"]["MinVelocity"] = Rotation.MinVelocity;
		J["Rotation"]["MaxVelocity"] = Rotation.MaxVelocity;
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

	static void operator<<(JSON& J, const ParticleModuleColor& Color)
	{
		J["Color"]["Min"] = Color.Min;
		J["Color"]["Max"] = Color.Max;

		for (uint32 i = 0; i < Color.Curve.Points.size(); i++)
		{
			J["Color"]["Curve"][i]["Key"] = Color.Curve.Points[i].Key;
			J["Color"]["Curve"][i]["Color"] = Color.Curve.Points[i].Value;
		}

		switch (Color.Mode)
		{
		case ParticleModuleColor::UpdateMode::Initial: J["Color"]["Mode"] = "Initial"; break;
		case ParticleModuleColor::UpdateMode::OverLife: J["Color"]["Mode"] = "OverLife"; break;
		}
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

	static void operator<<(JSON& J, const ParticleModuleSize& Size)
	{
		J["Size"]["Min"] = Size.Min;
		J["Size"]["Max"] = Size.Max;

		for (uint32 i = 0; i < Size.Curve.Points.size(); i++)
		{
			J["Size"]["Curve"][i]["Key"] = Size.Curve.Points[i].Key;
			J["Size"]["Curve"][i]["Size"] = Size.Curve.Points[i].Value;
		}

		switch (Size.Mode)
		{
		case ParticleModuleSize::UpdateMode::Initial: J["Size"]["Mode"] = "Initial"; break;
		case ParticleModuleSize::UpdateMode::OverLife: J["Size"]["Mode"] = "OverLife"; break;
		}
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

	static void operator<<(JSON& J, const ParticleModuleSubUV& SubUV)
	{
		J["SubUV"]["Horizontal"] = SubUV.Horizontal;
		J["SubUV"]["Vertical"] = SubUV.Vertical;
		J["SubUV"]["Cycles"] = SubUV.Cycles;

		switch (SubUV.Mode)
		{
		case ParticleModuleSubUV::SubUVMode::Linear: J["SubUV"]["Mode"] = "Linear"; break;
		case ParticleModuleSubUV::SubUVMode::Random: J["SubUV"]["Mode"] = "Random"; break;
		}
	}

	void ParticleEmitterCPU::Serialize(JSON& J) const
	{
		J["Visible"] = Visible;
		J["Emit"] = Emit;
		J["MaxCount"] = MaxParticles;
		J["EmitRate"] = EmitRate;
		J << Blend;
		J << Billboard;
		J << Transformation;
		J << Sort;
		
		J << ModuleLifetime;
		J << ModuleLocation;
		J << ModuleVelocity;
		J << ModuleRotation;
		J << ModuleColor;
		J << ModuleSize;
		//TODO: Noise
		J << ModuleSubUV;
	}

	void ParticleEmitterCPU::Deserialize(JSON& J)
	{
		Visible = J["Visible"].GetBool();
		Emit = J["Emit"].GetBool();
		MaxParticles = (uint32)J["MaxCount"].GetInt();
		EmitRate = (float)J["EmitRate"].GetFloat();

		J >> Blend;
		J >> Billboard;
		J >> Transformation;
		J >> Sort;
		
		J >> ModuleLifetime;
		J >> ModuleLocation;
		J >> ModuleVelocity;
		J >> ModuleRotation;
		J >> ModuleColor;
		J >> ModuleSize;
		//TODO: Noise
		J >> ModuleSubUV;
	}

}


