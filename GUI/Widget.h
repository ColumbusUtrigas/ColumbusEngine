#pragma once

#include <Math/Vector2.h>
#include <Math/Vector3.h>
#include <Math/Vector4.h>

#include <GUI/IO.h>

namespace C
{

  namespace GUI
  {

    class C_Widget
    {
    protected:
      C_Vector2 mPos = C_Vector2(0, 0);
      C_Vector2 mSize = C_Vector2(0.2, 0.1);

      int mState = 0;
      C_IO mIO;
    public:
      C_Widget() {}

      void setIO(C_IO& aIO) { mIO = aIO; }

      virtual void update() = 0;
      virtual void draw() = 0;

      ~C_Widget() {}
    };

  }

}
