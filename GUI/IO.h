#pragma once

namespace Columbus
{

	namespace GUI
	{

		struct C_Mouse
		{
			Vector2 coords;
			bool enabled = true;
			bool left = false;
			bool middle = false;
			bool right = false;
		};

		struct C_Screen
		{
			Vector2 size;
			float aspect = 1.0;
		};

		struct C_IO
		{
			C_Mouse mouse;
			C_Screen screen;
		};

	}

}
