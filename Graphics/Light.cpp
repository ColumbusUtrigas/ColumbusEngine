#include <Graphics/Light.h>
#include <System/Serializer.h>
#include <System/Log.h>
#include <cstring>

namespace Columbus
{

	Light::Light() {}
	
	Light::Light(int Type, const Vector3& Pos) :
		Type(Type),
		Pos(Pos) {}
	
	Light::Light(const char* File, const Vector3& Pos) :
		Pos(Pos)
	{
		Load(File);
	}
	
	bool Light::SaveToXML(const char* aFile) const
	{
		Serializer::SerializerXML Serializer;

		if (!Serializer.Write(aFile, "Light"))
		{ Log::Error("Can't save Light: %s", aFile); return false; }

		if (!Serializer.SetInt("Type", Type))
		{ Log::Error("Can't save Light type: %s", aFile); return false; }

		if (!Serializer.SetVector3("Direction", Dir, { "X", "Y", "Z" }))
		{ Log::Error("Can't save Light direction: %s", aFile); return false; }

		if (!Serializer.SetVector3("Color", Color, { "R", "G", "B" }))
		{ Log::Error("Can't save Light color: %s", aFile); return false; }

		if (!Serializer.SetFloat("Constant", Constant))
		{ Log::Error("Can't save Light constant: %s", aFile); return false; }

		if (!Serializer.SetFloat("Linear", Linear))
		{ Log::Error("Can't save Light linear: %s", aFile); return false; }

		if (!Serializer.SetFloat("Quadratic", Quadratic))
		{ Log::Error("Can't save Light quadratic: %s", aFile); return false; }

		if (!Serializer.SetFloat("InnerCutoff", InnerCutoff))
		{ Log::Error("Can't save Light inner cutoff: %s", aFile); return false; }

		if (!Serializer.SetFloat("OuterCutoff", OuterCutoff))
		{ Log::Error("Can't save Light outer cutoff: %s", aFile); return false; }

		if (!Serializer.Save())
		{ Log::Error("Can't save Light: %s", aFile); return false; }

		Log::Success("Light saved: %s", aFile);

		return true;
	}
	
	bool Light::LoadFromXML(const char* aFile)
	{
		Serializer::SerializerXML Serializer;

		if (!Serializer.Read(aFile, "Light"))
		{ Log::Error("Can't load Light: %s", aFile); return false; }

		if (!Serializer.GetInt("Type", Type))
		{ Log::Error("Can't load Light type: %s", aFile); return false; }

		if (!Serializer.GetVector3("Direction", Dir, { "X", "Y", "Z" }))
		{ Log::Error("Can't load Light direction: %s", aFile); return false; }

		if (!Serializer.GetVector3("Color", Color, { "R", "G", "B" }))
		{ Log::Error("Can't load Light color: %s", aFile); return false; }

		if (!Serializer.GetFloat("Constant", Constant))
		{ Log::Error("Can't load Light constant: %s", aFile); return false; }

		if (!Serializer.GetFloat("Linear", Linear))
		{ Log::Error("Can't load Light linear: %s", aFile); return false; }

		if (!Serializer.GetFloat("Quadratic", Quadratic))
		{ Log::Error("Can't load Light quadratic: %s", aFile); return false; }

		if (!Serializer.GetFloat("InnerCutoff", InnerCutoff))
		{ Log::Error("Can't load Light inner cutoff: %s", aFile); return false; }

		if (!Serializer.GetFloat("OuterCutoff", InnerCutoff))
		{ Log::Error("Can't load Light outer cutoff: %s", aFile); return false; }

		Log::Success("Light loaded: %s", aFile);

		return true;
	}
	
	bool Light::Load(const char* File)
	{
		if (strstr(File, ".cxlig") != nullptr)
		{
			return LoadFromXML(File);
		}

		return false;
	}
	
	Light::~Light() {}

}


