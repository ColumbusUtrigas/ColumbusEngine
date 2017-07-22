/************************************************
*                Primitives.cpp                 *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*             Nikolay(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#include <Graphics/Primitives.h>

namespace C
{
  //////////////////////////////////////////////////////////////////////////////
  //Generate plane
  std::vector<C_Vertex> C_PrimitivePlane(C_Vector2 aSize)
  {
    float verts[18] =
    {
      (float)0.5 * aSize.x, 0.0, (float)-0.5 * aSize.y,
      (float)-0.5 * aSize.x, 0.0, (float)-0.5 * aSize.y,
      (float)-0.5 * aSize.x, 0.0, (float)0.5 * aSize.y,

      (float)-0.5 * aSize.x, 0.0, (float)0.5 * aSize.y,
      (float)0.5 * aSize.x, 0.0, (float)0.5 * aSize.y,
      (float)0.5 * aSize.x, 0.0, (float)-0.5 * aSize.y
    };

    float uvs[12] =
    {
      1.0, 1.0,
      0.0, 1.0,
      0.0, 0.0,

      0.0, 0.0,
      1.0, 0.0,
      1.0, 1.0
    };

    float norms[18] =
    {
      0.0, 1.0, 0.0,
      0.0, 1.0, 0.0,
      0.0, 1.0, 0.0,

      0.0, 1.0, 0.0,
      0.0, 1.0, 0.0,
      0.0, 1.0, 0.0
    };

    std::vector<C_Vertex> v;

    for (size_t i = 0; i < 6; i++)
    {
      C_Vertex a;
      a.pos = C_Vector3(verts[0 + i * 3], verts[1 + i * 3], verts[2 + i * 3]);
      a.UV = C_Vector2(uvs[0 + i * 2], uvs[1 + i * 2]);
      a.normal = C_Vector3(norms[0 + i * 3], norms[1 + i * 3], norms[2 + i * 3]);
      v.push_back(a);
    };

    return v;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Generate box
  std::vector<C_Vertex> C_PrimitiveBox(C_Vector3 aSize)
  {
    const char* f =
    {
      "o Cube_Cube.001\n"
      "v -1.000000 -1.000000 1.000000\n"
      "v -1.000000 1.000000 1.000000\n"
      "v -1.000000 -1.000000 -1.000000\n"
      "v -1.000000 1.000000 -1.000000\n"
      "v 1.000000 -1.000000 1.000000\n"
      "v 1.000000 1.000000 1.000000\n"
      "v 1.000000 -1.000000 -1.000000\n"
      "v 1.000000 1.000000 -1.000000\n"
      "vt 1.000000 0.000000\n"
      "vt 1.000000 1.000000\n"
      "vt 0.000000 1.000000\n"
      "vt 0.000000 0.000000\n"
      "vn -1.000000 0.000000 0.000000\n"
      "vn 0.000000 0.000000 -1.000000\n"
      "vn 1.000000 0.000000 0.000000\n"
      "vn 0.000000 0.000000 1.000000\n"
      "vn 0.000000 -1.000000 0.000000\n"
      "vn 0.000000 1.000000 0.000000\n"
      "f 4/1/1 3/2/1 1/3/1\n"
      "f 8/1/2 7/2/2 3/3/2\n"
      "f 6/1/3 5/2/3 7/3/3\n"
      "f 2/1/4 1/2/4 5/3/4\n"
      "f 3/1/5 7/2/5 5/3/5\n"
      "f 8/1/6 4/2/6 2/3/6\n"
      "f 2/4/1 4/1/1 1/3/1\n"
      "f 4/4/2 8/1/2 3/3/2\n"
      "f 8/4/3 6/1/3 7/3/3\n"
      "f 6/4/4 2/1/4 5/3/4\n"
      "f 1/4/5 3/1/5 5/3/5\n"
      "f 6/4/6 8/1/6 2/3/6\n"
    };

    C_WriteFile("tmp_primitive_box.obj", f);
    std::vector<C_Vertex> v = Importer::C_LoadOBJVertices("tmp_primitive_box.obj");
    std::vector<C_Vertex> ret;

    for (size_t i = 0; i < v.size(); i++)
    {
      C_Vertex a = v[i];
      a.pos.x *= aSize.x;
      a.pos.y *= aSize.y;
      a.pos.z *= aSize.z;
      ret.push_back(a);
    }

    v.clear();

    C_DeleteFile("tmp_primitive_box.obj");
    return ret;
  }
  //////////////////////////////////////////////////////////////////////////////
  //Generate sphere
  std::vector<C_Vertex> C_PrimitiveSphere(float aRadius, size_t aRings, size_t aSectors, C_Vector3 aSize)
  {
    std::vector<C_Vertex> v;

    return v;
  }

}
