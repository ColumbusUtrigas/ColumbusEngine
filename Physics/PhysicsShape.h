#pragma once

#include <Core/Reflection.h>
#include <Scene/Transform.h>
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

	enum class ECollisionMotionType
	{
		Static,
		Kinematic,
		Dynamic,
	};

	struct HCollisionShapeDesc
	{
		ECollisionShape Type = ECollisionShape::None;
		Transform LocalTransform;

		float Radius = 0.5f; // Radius is used for Sphere, Cone, Capsule, Cylinder
		float Height = 1.5f; // Height is used for Cone, Capsule, Cylinder
		float Margin = 0.01f; // Collision skin in meters for Bullet convex shapes

		Vector3 Size = Vector3(1, 1, 1); // Size is used for Box

		std::vector<HCollisionShapeDesc> ChildShapes; // only used for Compound
	};

	// settings for the mesh collisions
	struct HCollisionSettings
	{
		ECollisionMotionType MotionType = ECollisionMotionType::Static;
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
		bool    bEnableCCD      = false;
		float   CCDMotionThreshold = 0.05f;
		float   CCDSweptSphereRadius = 0.02f;

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
CREFLECT_DECLARE_ENUM  (Columbus::ECollisionMotionType, "8B65DF4E-3828-4D8C-B4B8-9EEA34734EE5");
CREFLECT_DECLARE_STRUCT(Columbus::HCollisionShapeDesc, 2, "53FBBA2B-82CD-42A0-AE4F-E1B6C2A53409");
CREFLECT_DECLARE_STRUCT(Columbus::HCollisionSettings,  3, "D64028D3-5872-476F-AB40-F74A0A835B53");
