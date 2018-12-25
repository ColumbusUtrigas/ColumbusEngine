#include <Graphics/Light.h>
#include <System/Log.h>
#include <System/Serializer.h>

namespace Columbus
{

	Light::Light() {}
	
	Light::Light(int Type, const Vector3& Pos) :
		Type(Type),
		Pos(Pos) {}
	
	Light::Light(const std::string& File, const Vector3& Pos) :
		Pos(Pos)
	{
		Load(File);
	}
	
	bool Light::SaveToXML(const std::string& aFile) const
	{
		Serializer::SerializerXML Serializer;

		if (!Serializer.Write(aFile, "Light"))
		{ Log::error("Can't save Light: " + aFile); return false; }

		if (!Serializer.SetInt("Type", Type))
		{ Log::error("Can't save Light type: " + aFile); return false; }

		if (!Serializer.SetVector3("Direction", Dir, { "X", "Y", "Z" }))
		{ Log::error("Can't save Light direction: " + aFile); return false; }

		if (!Serializer.SetVector3("Color", Color, { "R", "G", "B" }))
		{ Log::error("Can't save Light color: " + aFile); return false; }

		if (!Serializer.SetFloat("Constant", Constant))
		{ Log::error("Can't save Light constant: " + aFile); return false; }

		if (!Serializer.SetFloat("Linear", Linear))
		{ Log::error("Can't save Light linear: " + aFile); return false; }

		if (!Serializer.SetFloat("Quadratic", Quadratic))
		{ Log::error("Can't save Light quadratic: " + aFile); return false; }

		if (!Serializer.SetFloat("InnerCutoff", InnerCutoff))
		{ Log::error("Can't save Light inner cutoff: " + aFile); return false; }

		if (!Serializer.SetFloat("OuterCutoff", OuterCutoff))
		{ Log::error("Can't save Light outer cutoff: " + aFile); return false; }

		if (!Serializer.Save())
		{ Log::error("Can't save Light: " + aFile); return false; }

		Log::success("Light saved: " + aFile);

		return true;
	}
	
	bool Light::LoadFromXML(const std::string& aFile)
	{
		Serializer::SerializerXML Serializer;

		if (!Serializer.Read(aFile, "Light"))
		{ Log::error("Can't load Light: " + aFile); return false; }

		if (!Serializer.GetInt("Type", Type))
		{ Log::error("Can't load Light type: %s" + aFile); return false; }

		if (!Serializer.GetVector3("Direction", Dir, { "X", "Y", "Z" }))
		{ Log::error("Can't load Light direction: %s" + aFile); return false; }

		if (!Serializer.GetVector3("Color", Color, { "R", "G", "B" }))
		{ Log::error("Can't load Light color: " + aFile); return false; }

		if (!Serializer.GetFloat("Constant", Constant))
		{ Log::error("Can't load Light constant: " + aFile); return false; }

		if (!Serializer.GetFloat("Linear", Linear))
		{ Log::error("Can't load Light linear: " + aFile); return false; }

		if (!Serializer.GetFloat("Quadratic", Quadratic))
		{ Log::error("Can't load Light quadratic: " + aFile); return false; }

		if (!Serializer.GetFloat("InnerCutoff", InnerCutoff))
		{ Log::error("Can't load Light inner cutoff: " + aFile); return false; }

		if (!Serializer.GetFloat("OuterCutoff", InnerCutoff))
		{ Log::error("Can't load Light outer cutoff: " + aFile); return false; }

		Log::success("Light loaded: " + aFile);

		return true;
	}
	
	bool Light::Load(const std::string& File)
	{
		if (File.find_last_of(".cxlig") != std::string::npos)
		{
			return LoadFromXML(File);
		}

		return false;
	}
	
	Light::~Light() {}

}
