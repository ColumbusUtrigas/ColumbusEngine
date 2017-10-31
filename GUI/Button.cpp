#include <GUI/Button.h>
#include <cstdio>

namespace C
{

	namespace GUI
	{

		void C_Button::update()
		{
			C_Vector2 pos = mPos * mIO.screen.size;
			C_Vector2 size = mSize * mIO.screen.size;
		}

		void C_Button::draw()
		{

		}

	}

}
