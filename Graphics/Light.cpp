#include <Graphics/Light.h>
#include <Common/JSON/JSON.h>

namespace Columbus
{

	void Light::Serialize(JSON& J) const
	{
		J["Color"] = Color;
		J["Energy"] = Energy;
		J["Range"] = Range;
		J["InnerCutoff"] = InnerCutoff;
		J["OuterCutoff"] = OuterCutoff;

		switch (Type)
		{
		case Light::Directional: J["Type"] = "Directional"; break;
		case Light::Point:       J["Type"] = "Point";       break;
		case Light::Spot:        J["Type"] = "Spot";        break;
		}
	}

	void Light::Deserialize(JSON& J)
	{
		Color = J["Color"].GetVector3<float>();
		Energy = (float)J["Energy"].GetFloat();
		Range = (float)J["Range"].GetFloat();
		InnerCutoff = (float)J["InnerCutoff"].GetFloat();
		OuterCutoff = (float)J["OuterCutoff"].GetFloat();

		if (J["Type"].GetString() == "Directional")
			Type = Light::Directional;
		else if (J["Type"].GetString() == "Point")
			Type = Light::Point;
		else if (J["Type"].GetString() == "Spot")
			Type = Light::Spot;
	}

}

