#pragma once

namespace Columbus
{

	// TODO: LEGACY
	struct Billboard
	{
		enum RotateMode
		{
			LocalX,
			LocalY,
			LocalZ
		};

		RotateMode Rotate = LocalY;
		int a = 123;
	};

}
