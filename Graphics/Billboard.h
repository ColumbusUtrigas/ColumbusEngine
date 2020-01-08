#pragma once

namespace Columbus
{

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
