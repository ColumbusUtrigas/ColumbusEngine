#include <Graphics/Light.h>
#include <Common/JSON/JSON.h>
#include <System/Log.h>

namespace Columbus
{

	Light::Light() {}
	Light::Light(int Type, const Vector3& Pos) : Type(Type), Pos(Pos) {}
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

		if (Type == 0) Dir = J["Directional"]["Direction"].GetVector3<float>();
		if (Type == 1) Range = (float)J["Point"]["Range"].GetFloat();

		Log::Success("Light has been loaded: %s", FileName);

		return true;
	}
	
	Light::~Light() {}

}


