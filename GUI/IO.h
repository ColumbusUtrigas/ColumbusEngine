#pragma once

namespace C
{

	namespace GUI
	{

		struct C_Mouse
		{
			C_Vector2 coords;
			bool left = false;
			bool middle = false;
			bool right = false;
		};

		struct C_Screen
		{
			C_Vector2 size;
			float aspect = 1.0;
		};

		struct C_IO
		{
			C_Mouse mouse;
			C_Screen screen;
		};

	}

}
