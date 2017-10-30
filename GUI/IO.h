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

      C_Mouse& operator=(C_Mouse& other)
      {
		  coords = other.coords;
		  left = other.left;
		  middle = other.middle;
		  right = other.right;
		  return *this;
	  }
    };

    struct C_Screen
    {
      C_Vector2 size;
      float aspect = 1.0;
      
      C_Screen& operator=(C_Screen& other)
      {
		  size = other.size;
		  aspect = other.aspect;
		  return *this;
	  }
    };

    struct C_IO
    {
      C_Mouse mouse;
      C_Screen screen;

      C_IO& operator=(C_IO& other)
	  {
		  mouse = other.mouse;
		  screen = other.screen;
		  return *this;
	  }
    };

  }

}
