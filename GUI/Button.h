/************************************************
*                  Button.h                     *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   28.10.2017                  *
*************************************************/

#pragma once

#include <GUI/Widget.h>

namespace Columbus
{

	namespace GUI
	{

		class Button : public Widget
		{
		public:
			Button() {}

			void update() override;
			void draw() override;

			~Button() {}
		};

	}

}
