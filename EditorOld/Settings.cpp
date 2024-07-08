#include <Editor/Settings.h>
#include <Common/JSON/JSON.h>

namespace Columbus::Editor
{

	void Settings::TryLoad()
	{
		JSON j;
		j.Load("editor_settings.json");

		windowSize = j["windowSize"];
		windowMaximized = (bool)j["windowMaximized"];
		sceneView.icons = (bool)j["sceneView"]["icons"];
		sceneView.grid = (bool)j["sceneView"]["grid"];
		sceneView.gizmo = (bool)j["sceneView"]["gizmo"];
		sceneView.timeFactor = (float)j["sceneView"]["timeFactor"];
	}

	void Settings::TrySave() const
	{
		JSON j;

		j["windowSize"] = windowSize;
		j["windowMaximized"] = windowMaximized;
		j["sceneView"]["icons"] = sceneView.icons;
		j["sceneView"]["grid"] = sceneView.grid;
		j["sceneView"]["gizmo"] = sceneView.gizmo;
		j["sceneView"]["timeFactor"] = sceneView.timeFactor;

		j.Save("editor_settings.json");
	}

}
