#include <Editor/Settings.h>
#include <Common/JSON/JSON.h>

namespace Columbus::Editor
{

	void Settings::Serialize(JSON& J) const
	{
		J["windowSize"] = windowSize;
		J["windowMaximized"] = windowMaximized;
	}

	void Settings::Deserialize(JSON& J)
	{
		windowSize = J["windowSize"];
		windowMaximized = (bool)J["windowMaximized"];
	}

}
