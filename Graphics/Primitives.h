#pragma once

#include <Graphics/Importer.h>
#include <System/System.h>

namespace C
{

  std::vector<C_Vertex> C_PrimitivePlane(C_Vector2 aSize = C_Vector2(1.0, 1.0));

  std::vector<C_Vertex> C_PrimitiveBox(C_Vector3 aSize = C_Vector3(1.0, 1.0, 1.0));

}
