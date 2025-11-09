#pragma once

#include <Core/Reflection.h>
#include <Math/Vector3.h>
#include <vector>

#include <btBulletDynamicsCommon.h>

namespace Columbus
{

	enum class ECollisionShape
	{
		None,
		Box,
		Sphere,
		Capsule,
		Cone,
		Cylinder,
		ConvexHull,
		TriMesh,
		Compound,
	};

	struct HCollisionShapeDesc
	{
		ECollisionShape Type = ECollisionShape::None;

		float Radius = 0.5f; // Radius is used for Sphere, Cone, Capsule
		float Height = 1.5f; // Height is used for Cone, Capsule

		Vector3 Size = Vector3(1, 1, 1); // Size is used for Box, Cylinder

		std::vector<HCollisionShapeDesc> ChildShapes; // only used for Compound
	};

	// settings for the mesh collisions
	struct HCollisionSettings
	{
		bool    Static          = true;
		float   Mass            = 1.0f;
		float   Restitution     = 0.0f;
		float   Friction        = 0.5f;
		float   RollingFriction = 0.0f;
		float   AngularDamping  = 0.2f;
		float   AngularTreshold = 0.25f;
		Vector3 AngularFactor   = Vector3(1, 1, 1);
		float   LinearTreshold  = 0.2f;
		float   LinearDamping   = 0.2f;
		Vector3 LinearFactor    = Vector3(1, 1, 1);

		HCollisionShapeDesc Shape;
	};

	struct Mesh2;

	// helper functions
	namespace Physics
	{
		using namespace Columbus;
		void CombineAllPointsFromMesh(Columbus::Mesh2* Mesh, std::vector<float>& OutPoints);

		btCollisionShape* CreatePhysicsShapeFromDesc(const HCollisionShapeDesc& Desc, Mesh2* Mesh);
	}
}

// reflection declarations
CREFLECT_DECLARE_ENUM  (Columbus::ECollisionShape, "24B03462-DAA7-40E9-B2C2-0858CA66566F");
CREFLECT_DECLARE_STRUCT(Columbus::HCollisionShapeDesc, 1, "53FBBA2B-82CD-42A0-AE4F-E1B6C2A53409");
CREFLECT_DECLARE_STRUCT(Columbus::HCollisionSettings,  1, "D64028D3-5872-476F-AB40-F74A0A835B53");
