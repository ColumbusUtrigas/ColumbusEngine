#pragma once

#include <Core/ISerializable.h>
#include <Math/Vector2.h>

namespace Columbus::Editor
{

	class Settings : public ISerializable
	{
	public:
		iVector2 windowSize = { 640, 480 };
		bool windowMaximized = false;

		struct
		{
			bool icons = true;
			bool grid = true;
			bool gizmo = true;
			float timeFactor = 1.0f;
		} sceneView;

		void Serialize(JSON& J) const override;
		void Deserialize(JSON& J) override;
	};

}
