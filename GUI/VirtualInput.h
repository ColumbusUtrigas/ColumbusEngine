#pragma once

namespace Columbus
{

	struct VirtualMouse
	{
		Vector2 Coords;
		bool Enabled = true;
		bool Left = false;
		bool Middle = false;
		bool Right = false;
	};

	struct VirtualScreen
	{
		Vector2 Size;
		float Aspect = 1.0;
	};

	struct VirtualInput
	{
		VirtualMouse Mouse;
		VirtualScreen Screen;
	};

}
