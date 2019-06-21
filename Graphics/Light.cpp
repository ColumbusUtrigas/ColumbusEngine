#include <Graphics/Light.h>
#include <Common/JSON/JSON.h>
#include <System/Log.h>

namespace Columbus
{

	Light::Light() {}
	Light::Light(int Type, const Vector3& Pos) : Pos(Pos), Type(Type) {}
	Light::Light(const char* File, const Vector3& Pos) : Pos(Pos)
	{
		Load(File);
	}

	bool Light::Load(const char* FileName)
	{
		auto Error = [FileName]() { Log::Error("Light has not been loaded: %s", FileName); };

		JSON J;
		if (!J.Load(FileName)) { Error(); return false; }

		Type = (int)J["Type"].GetInt();
		Color = J["Color"].GetVector3<float>();

		Energy = (float)J["Energy"].GetFloat();

		if (Type == 0) Dir = J["Directional"]["Direction"].GetVector3<float>();
		if (Type == 1) Range = (float)J["Point"]["Range"].GetFloat();
		if (Type == 2)
		{
			InnerCutoff = (float)J["Spot"]["Inner"].GetFloat();
			OuterCutoff = (float)J["Spot"]["Outer"].GetFloat();
		}

		Log::Success("Light has been loaded: %s", FileName);

		return true;
	}
	
	Light::~Light() {}

}


