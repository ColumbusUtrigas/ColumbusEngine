#pragma once

#include <Common/Model/Model.h>
#include <Core/Types.h>
#include <vector>

namespace Columbus
{
	std::vector<Vertex> PrimitivePlane(const Vector3& Size = Vector3(1.0, 1.0, 1.0));
	std::vector<Vertex> PrimitiveBox(const Vector3& Size = Vector3(1.0, 1.0, 1.0));
	std::vector<Vertex> PrimitiveSphere(float Radius, uint32 Rings, uint32 Sectors);
	std::vector<Vertex> PrimitiveCone(float Base, float Height, uint32 Slices, uint32 Stacks);
	std::vector<Vertex> PrimitiveTorus(float Inner, float Outer, uint32 Sides, uint32 Rings);
	std::vector<Vertex> PrimitiveCylinder(float Radius, float Height, uint32 Slices, uint32 Stacks);

}
