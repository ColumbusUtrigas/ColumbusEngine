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
	std::vector<C_Vertex> C_PrimitiveSphere(float aRadius, size_t aRings, size_t aSectors, C_Vector3 aSize = C_Vector3(1.0, 1.0, 1.0));

}
