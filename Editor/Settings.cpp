#include <Editor/Settings.h>
#include <Common/JSON/JSON.h>

namespace Columbus::Editor
{

	void Settings::Serialize(JSON& J) const
	{
		J["windowSize"] = windowSize;
		J["windowMaximized"] = windowMaximized;
		J["sceneView"]["icons"] = sceneView.icons;
		J["sceneView"]["grid"] = sceneView.grid;
		J["sceneView"]["gizmo"] = sceneView.gizmo;
		J["sceneView"]["timeFactor"] = sceneView.timeFactor;
	}

	void Settings::Deserialize(JSON& J)
	{
		windowSize = J["windowSize"];
		windowMaximized = (bool)J["windowMaximized"];
		sceneView.icons = (bool)J["sceneView"]["icons"];
		sceneView.grid = (bool)J["sceneView"]["grid"];
		sceneView.gizmo = (bool)J["sceneView"]["gizmo"];
		sceneView.timeFactor = (float)J["sceneView"]["timeFactor"];
	}

}
