/************************************************
*                 Primitives.h                  *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#pragma once

#include <System/Importer.h>
#include <System/System.h>

namespace Columbus
{
	//Generate plane
	std::vector<C_Vertex> C_PrimitivePlane(C_Vector3 aSize = C_Vector3(1.0, 1.0, 1.0));
	//Generate box
	std::vector<C_Vertex> C_PrimitiveBox(C_Vector3 aSize = C_Vector3(1.0, 1.0, 1.0));
	//Generate sphere
	std::vector<C_Vertex> C_PrimitiveSphere(float aRadius, size_t aRings, size_t aSectors);
	//Generate cone
	std::vector<C_Vertex> c_PrimitiveCone(const float aBase, const float aHeight, const size_t aSlices, const size_t aStacks);
	//Generate torus
	std::vector<C_Vertex> C_PrimitiveTorus(const float aInner, const float aOuter, const size_t aSides, const size_t aRings);
	//Generate cylinder
	std::vector<C_Vertex> C_PrimitiveCylinder(const float aRadius, const float aHeight, const size_t aSlices, const size_t aStacks);

}
