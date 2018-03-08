#pragma once

namespace Columbus
{

	namespace GUI
	{

		struct VirtualMouse
		{
			Vector2 coords;
			bool enabled = true;
			bool left = false;
			bool middle = false;
			bool right = false;
		};

		struct VirtualScreen
		{
			Vector2 size;
			float aspect = 1.0;
		};

		struct IO
		{
			VirtualMouse mouse;
			VirtualScreen screen;
		};

	}

}
