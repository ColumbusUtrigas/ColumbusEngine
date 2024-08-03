#pragma once

#include <Math/Vector2.h>

namespace Columbus::Editor
{

	struct Settings
	{
		iVector2 windowSize = { 640, 480 };
		bool windowMaximized = false;

		struct
		{
			bool icons = true;
			bool grid = true;
			bool gizmo = true;
			float timeFactor = 1.0f;
		} sceneView;

		void TryLoad();
		void TrySave() const;
	};

}
