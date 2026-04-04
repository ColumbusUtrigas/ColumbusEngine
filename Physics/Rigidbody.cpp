#include <Physics/Rigidbody.h>
#include <Graphics/Mesh.h>

#include <BulletCollision/CollisionShapes/btShapeHull.h>

namespace Columbus
{

	Rigidbody::Rigidbody(btCollisionShape* InShape)
	{
		btDefaultMotionState* MotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));
		btRigidBody::btRigidBodyConstructionInfo* CI = new btRigidBody::btRigidBodyConstructionInfo(1, MotionState, new btSphereShape(1), btVector3(0, 0, 0));
		mRigidbody = new btRigidBody(*CI);

		SetAngularDamping(AngularDamping);
		SetLinearDamping(LinearDamping);
		SetAngularTreshold(AngularTreshold);
		SetLinearTreshold(LinearTreshold);

		mRigidbody->setCollisionShape(InShape);
		SetStatic(Static);
	}

	void Rigidbody::SetCollisionSettings(const HCollisionSettings& Settings)
	{
		SetMass(Settings.Mass);
		SetRestitution(Settings.Restitution);
		SetFriction(Settings.Friction);
		SetRollingFriction(Settings.RollingFriction);
		SetAngularDamping(Settings.AngularDamping);
		SetAngularTreshold(Settings.AngularTreshold);
		SetAngularFactor(Settings.AngularFactor);
		SetLinearTreshold(Settings.LinearTreshold);
		SetLinearDamping(Settings.LinearDamping);
		SetLinearFactor(Settings.LinearFactor);
		SetStatic(Settings.Static);
	}

	void Rigidbody::Activate()
	{
		if (mRigidbody != nullptr)
		{
			mRigidbody->activate();
		}
	}

	void Rigidbody::ApplyCentralForce(Vector3 Force)
	{
		if (mRigidbody != nullptr)
		{
			mRigidbody->activate();
			mRigidbody->applyCentralForce(btVector3(Force.X, Force.Y, Force.Z));
		}
	}

	void Rigidbody::ApplyCentralImpulse(Vector3 Impulse)
	{
		if (mRigidbody != nullptr)
		{
			mRigidbody->activate();
			mRigidbody->applyCentralImpulse(btVector3(Impulse.X, Impulse.Y, Impulse.Z));
		}
	}

	void Rigidbody::ApplyForce(Vector3 Force, Vector3 RelPos)
	{
		if (mRigidbody != nullptr)
		{
			mRigidbody->activate();
			mRigidbody->applyForce(btVector3(Force.X, Force.Y, Force.Z), btVector3(RelPos.X, RelPos.Y, RelPos.Z));
		}
	}

	void Rigidbody::ApplyImpulse(Vector3 Impulse, Vector3 RelPos)
	{
		if (mRigidbody != nullptr)
		{
			mRigidbody->activate();
			mRigidbody->applyImpulse(btVector3(Impulse.X, Impulse.Y, Impulse.Z), btVector3(RelPos.X, RelPos.Y, RelPos.Z));
		}
	}

	void Rigidbody::ApplyTorque(Vector3 Torque)
	{
		if (mRigidbody != nullptr)
		{
			mRigidbody->activate();
			mRigidbody->applyTorque(btVector3(Torque.X, Torque.Y, Torque.Z));
		}
	}

	void Rigidbody::ApplyTorqueImpulse(Vector3 Torque)
	{
		if (mRigidbody != nullptr)
		{
			mRigidbody->activate();
			mRigidbody->applyTorqueImpulse(btVector3(Torque.X, Torque.Y, Torque.Z));
		}
	}

	void Rigidbody::SetStatic(bool InStatic)
	{
		if (mRigidbody != nullptr)
		{
			Static = InStatic;

			if (InStatic == true)
			{
				mRigidbody->setMassProps(0, btVector3(0, 0, 0));
				mRigidbody->setCollisionFlags(mRigidbody->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);
			} else
			{
				SetMass(Mass);
				SetAngularVelocity(Vector3(0, 0, 0));
				SetLinearVelocity(Vector3(0, 0, 0));
			}
		}
	}

	void Rigidbody::SetTransform(Transform InTransform)
	{
		if (mRigidbody != nullptr)
		{
			btTransform bTrans;
			Vector3 pos = InTransform.Position;

			Quaternion Q = InTransform.Rotation;
			btQuaternion bQuat(-Q.X, -Q.Y, -Q.Z, Q.W);
			btVector3 scale = btVector3(InTransform.Scale.X, InTransform.Scale.Y, InTransform.Scale.Z);

			bTrans.setOrigin(btVector3(pos.X, pos.Y, pos.Z));
			bTrans.setRotation(bQuat);

			//if (mRigidbody->isKinematicObject())
			mRigidbody->setWorldTransform(bTrans);
			mRigidbody->proceedToTransform(bTrans);

			mRigidbody->getCollisionShape()->setLocalScaling(scale);

			Trans = InTransform;
		}
	}

	void Rigidbody::SetMass(float InMass)
	{
		if (mRigidbody != nullptr)
		{
			Mass = InMass;
			
			if (!Static)
			{
				btVector3 Inertia;
				mRigidbody->getCollisionShape()->calculateLocalInertia(InMass, Inertia);
				mRigidbody->setMassProps(InMass, Inertia);
			}
		}
	}

	void Rigidbody::SetRestitution(float InRestitution)
	{
		if (mRigidbody != nullptr)
		{
			Restitution = InRestitution;
			mRigidbody->setRestitution(InRestitution);
		}
	}

	void Rigidbody::SetFriction(float InFriction)
	{
		if (mRigidbody != nullptr)
		{
			Friction = InFriction;
			mRigidbody->setFriction(InFriction);
		}
	}

	void Rigidbody::SetRollingFriction(float InFriction)
	{
		if (mRigidbody != nullptr)
		{
			RollingFriction = InFriction;
			mRigidbody->setRollingFriction(InFriction);
		}
	}

	void Rigidbody::SetAngularDamping(float InDamping)
	{
		if (mRigidbody != nullptr)
		{
			AngularDamping = InDamping;
			mRigidbody->setDamping(LinearDamping, AngularDamping);
		}
	}

	void Rigidbody::SetAngularTreshold(float InTreshold)
	{
		if (mRigidbody != nullptr)
		{
			AngularTreshold = InTreshold;
			mRigidbody->setSleepingThresholds(LinearTreshold, AngularTreshold);
		}
	}

	void Rigidbody::SetAngularFactor(Vector3 InFactor)
	{
		if (mRigidbody != nullptr)
		{
			AngularFactor = InFactor;
			mRigidbody->setAngularFactor(btVector3(InFactor.X, InFactor.Y, InFactor.Z));
		}
	}

	void Rigidbody::SetAngularVelocity(Vector3 Velocity)
	{
		if (mRigidbody != nullptr)
		{
			mRigidbody->activate();
			mRigidbody->setAngularVelocity(btVector3(Math::Radians(Velocity.X), Math::Radians(Velocity.Y), Math::Radians(Velocity.Z)));
		}
	}

	void Rigidbody::SetLinearDamping(float InDamping)
	{
		if (mRigidbody != nullptr)
		{
			LinearDamping = InDamping;
			mRigidbody->setDamping(LinearDamping, AngularDamping);
		}
	}

	void Rigidbody::SetLinearTreshold(float InTreshold)
	{
		if (mRigidbody != nullptr)
		{
			LinearTreshold = InTreshold;
			mRigidbody->setSleepingThresholds(LinearTreshold, AngularTreshold);
		}
	}

	void Rigidbody::SetLinearFactor(Vector3 InFactor)
	{
		if (mRigidbody != nullptr)
		{
			LinearFactor = InFactor;
			mRigidbody->setLinearFactor(btVector3(InFactor.X, InFactor.Y, InFactor.Z));
		}
	}

	void Rigidbody::SetLinearVelocity(Vector3 Velocity)
	{
		if (mRigidbody != nullptr)
		{
			mRigidbody->activate();
			mRigidbody->setLinearVelocity(btVector3(Velocity.X, Velocity.Y, Velocity.Z));
		}
	}

	void Rigidbody::SetGravity(Vector3 Gravity)
	{
		if (mRigidbody != nullptr)
		{
			mRigidbody->setGravity(btVector3(Gravity.X, Gravity.Y, Gravity.Z));
			mRigidbody->applyGravity();
		}
	}

	bool Rigidbody::IsStatic() const
	{
		return this->Static;
	}

	Transform Rigidbody::GetTransform() const
	{
		Transform Result;

		if (mRigidbody != nullptr)
		{
			btTransform bTrans;

			bTrans = mRigidbody->getWorldTransform();
			//mRigidbody->getMotionState()->getWorldTransform(bTrans);

			Result.Position = Vector3(bTrans.getOrigin().getX(), bTrans.getOrigin().getY(), bTrans.getOrigin().getZ());
			Result.Rotation = Quaternion(-bTrans.getRotation().getX(), -bTrans.getRotation().getY(), -bTrans.getRotation().getZ(), bTrans.getRotation().getW());
			Result.Scale = Trans.Scale;
			Result.Update();

			(Transform)this->Trans = Result;
		}

		return Result;
	}

	float Rigidbody::GetMass() const
	{
		return Mass;
	}

	float Rigidbody::GetRestitution() const
	{
		return Restitution;
	}

	float Rigidbody::GetFriction() const
	{
		return Friction;
	}

	float Rigidbody::GetRollingFriction() const
	{
		return RollingFriction;
	}

	float Rigidbody::GetAngularDamping() const
	{
		return AngularDamping;
	}

	float Rigidbody::GetAngularTreshold() const
	{
		return AngularTreshold;
	}

	Vector3 Rigidbody::GetAngularFactor() const
	{
		return AngularFactor;
	}

	Vector3 Rigidbody::GetAngularVelocity() const
	{
		if (mRigidbody != nullptr)
		{
			btVector3 Velocity = mRigidbody->getAngularVelocity();
			return Vector3(Velocity.getX(), Velocity.getY(), Velocity.getZ());
		}

		return Vector3(0, 0, 0);
	}

	float Rigidbody::GetLinearDamping() const
	{
		return LinearDamping;
	}

	float Rigidbody::GetLinearTreshold() const
	{
		return LinearTreshold;
	}

	Vector3 Rigidbody::GetLinearFactor() const
	{
		return LinearFactor;
	}

	Vector3 Rigidbody::GetLinearVelocity() const
	{
		if (mRigidbody != nullptr)
		{
			btVector3 Velocity = mRigidbody->getLinearVelocity();
			return Vector3(Velocity.getX(), Velocity.getY(), Velocity.getZ());
		}

		return Vector3(0, 0, 0);
	}

	Vector3 Rigidbody::GetGravity() const
	{
		if (mRigidbody != nullptr)
		{
			btVector3 Gravity = mRigidbody->getGravity();
			return Vector3(Gravity.getX(), Gravity.getY(), Gravity.getZ());
		}

		return Vector3(0, 0, 0);
	}

	Rigidbody::~Rigidbody()
	{
		delete mRigidbody->getMotionState();
		delete mRigidbody;
	}

}

// helper functions
namespace Columbus::Physics
{

	using namespace Columbus;

	void CombineAllPointsFromMesh(Mesh2* Mesh, std::vector<float>& OutPoints)
	{
		for (const MeshPrimitive& Prim : Mesh->Primitives)
		{
			for (const Vector3& Vert : Prim.CPU.Vertices)
			{
				OutPoints.push_back(Vert.X);
				OutPoints.push_back(Vert.Y);
				OutPoints.push_back(Vert.Z);
			}
		}
	}

	btCollisionShape* CreatePhysicsShapeFromDesc(const HCollisionShapeDesc& Desc, Mesh2* Mesh)
	{
		auto ToBulletTransformNoScale = [](const Transform& InTransform) -> btTransform
		{
			btTransform Result;
			Result.setIdentity();
			Result.setOrigin(btVector3(InTransform.Position.X, InTransform.Position.Y, InTransform.Position.Z));
			Result.setRotation(btQuaternion(InTransform.Rotation.X, InTransform.Rotation.Y, InTransform.Rotation.Z, InTransform.Rotation.W));
			return Result;
		};

		auto ToBulletScale = [](const Transform& InTransform) -> btVector3
		{
			return btVector3(fabsf(InTransform.Scale.X), fabsf(InTransform.Scale.Y), fabsf(InTransform.Scale.Z));
		};

		std::function<btCollisionShape*(const HCollisionShapeDesc&, bool)> CreateShapeInternal =
			[&](const HCollisionShapeDesc& InDesc, bool bIgnoreOwnPlacement) -> btCollisionShape*
		{
			btCollisionShape* Shape = nullptr;
			constexpr bool bOptimiseHull = true;

			switch (InDesc.Type)
			{
			case ECollisionShape::None: break;
			case ECollisionShape::Box:
				Shape = new btBoxShape(btVector3(InDesc.Size.X * 0.5f, InDesc.Size.Y * 0.5f, InDesc.Size.Z * 0.5f));
				break;
			case ECollisionShape::Sphere:
				Shape = new btSphereShape(InDesc.Radius);
				break;
			case ECollisionShape::Capsule:
				Shape = new btCapsuleShape(InDesc.Radius, InDesc.Height);
				break;
			case ECollisionShape::Cone:
				Shape = new btConeShape(InDesc.Radius, InDesc.Height);
				break;
			case ECollisionShape::Cylinder:
				Shape = new btCylinderShape(btVector3(InDesc.Size.X * 0.5f, InDesc.Size.Y * 0.5f, InDesc.Size.Z * 0.5f));
				break;
			case ECollisionShape::ConvexHull:
			{
				std::vector<float> AllPoints;
				CombineAllPointsFromMesh(Mesh, AllPoints);

				if (bOptimiseHull)
				{
					btConvexHullShape TmpHull(AllPoints.data(), AllPoints.size() / 3, sizeof(float) * 3);
					TmpHull.setMargin(0);

					btShapeHull* Hull = new btShapeHull(&TmpHull);
					Hull->buildHull(0);

					Shape = new btConvexHullShape((const btScalar*)Hull->getVertexPointer(), Hull->numVertices(), sizeof(btVector3));

					delete Hull;
				}
				else
				{
					Shape = new btConvexHullShape(AllPoints.data(), AllPoints.size() / 3, sizeof(float) * 3);
				}
				break;
			}
			case ECollisionShape::TriMesh:
			{
				btTriangleIndexVertexArray* va = new btTriangleIndexVertexArray();

				for (const MeshPrimitive& Prim : Mesh->Primitives)
				{
					const auto& verts = Prim.CPU.Vertices;
					const auto& inds = Prim.CPU.Indices;

					if (verts.empty() || inds.empty())
						continue;

					btIndexedMesh im;
					im.m_numTriangles = static_cast<unsigned int>(inds.size() / 3);
					im.m_triangleIndexBase = reinterpret_cast<const unsigned char*>(inds.data());
					im.m_triangleIndexStride = static_cast<int>(3 * sizeof(u32));
					im.m_numVertices = static_cast<unsigned int>(verts.size());
					im.m_vertexBase = reinterpret_cast<const unsigned char*>(verts.data());
					im.m_vertexStride = static_cast<int>(sizeof(Vector3));
					im.m_indexType = PHY_INTEGER;

					va->addIndexedMesh(im, PHY_INTEGER);
				}

				Shape = new btBvhTriangleMeshShape(va, true);
				break;
			}
			case ECollisionShape::Compound:
			{
				Shape = new btCompoundShape(true, (int)InDesc.ChildShapes.size());
				for (const HCollisionShapeDesc& Child : InDesc.ChildShapes)
				{
					btTransform bTransform = ToBulletTransformNoScale(Child.LocalTransform);
					static_cast<btCompoundShape*>(Shape)->addChildShape(bTransform, CreateShapeInternal(Child, true));
				}
				break;
			}
			}

			if (Shape != nullptr)
			{
				Shape->setLocalScaling(ToBulletScale(InDesc.LocalTransform));
			}

			if (Shape != nullptr && !bIgnoreOwnPlacement)
			{
				const bool bHasPlacement =
					fabsf(InDesc.LocalTransform.Position.X) >= 0.0001f ||
					fabsf(InDesc.LocalTransform.Position.Y) >= 0.0001f ||
					fabsf(InDesc.LocalTransform.Position.Z) >= 0.0001f ||
					fabsf(InDesc.LocalTransform.Rotation.X) >= 0.0001f ||
					fabsf(InDesc.LocalTransform.Rotation.Y) >= 0.0001f ||
					fabsf(InDesc.LocalTransform.Rotation.Z) >= 0.0001f ||
					fabsf(InDesc.LocalTransform.Rotation.W - 1.0f) >= 0.0001f;

				if (bHasPlacement)
				{
					btCompoundShape* Wrapper = new btCompoundShape(true, 1);
					Wrapper->addChildShape(ToBulletTransformNoScale(InDesc.LocalTransform), Shape);
					Shape = Wrapper;
				}
			}

			return Shape;
		};

		return CreateShapeInternal(Desc, false);
	}

} // namespace Columbus::Physics


// reflection stuff

using namespace Columbus;

CREFLECT_ENUM_BEGIN(ECollisionShape, "")
	CREFLECT_ENUM_FIELD(ECollisionShape::None,       0)
	CREFLECT_ENUM_FIELD(ECollisionShape::Box,        1)
	CREFLECT_ENUM_FIELD(ECollisionShape::Sphere,     2)
	CREFLECT_ENUM_FIELD(ECollisionShape::Capsule,    3)
	CREFLECT_ENUM_FIELD(ECollisionShape::Cone,       4)
	CREFLECT_ENUM_FIELD(ECollisionShape::Cylinder,   5)
	CREFLECT_ENUM_FIELD(ECollisionShape::ConvexHull, 6)
	CREFLECT_ENUM_FIELD(ECollisionShape::TriMesh,    7)
	CREFLECT_ENUM_FIELD(ECollisionShape::Compound,   8)
CREFLECT_ENUM_END()

CREFLECT_STRUCT_BEGIN(HCollisionShapeDesc, "")
	CREFLECT_STRUCT_FIELD(ECollisionShape, Type, "")
	CREFLECT_STRUCT_FIELD(Transform, LocalTransform, "")
	CREFLECT_STRUCT_FIELD(float, Radius, "")
	CREFLECT_STRUCT_FIELD(float, Height, "")
	CREFLECT_STRUCT_FIELD(Vector3, Size, "ColourChannels")
	CREFLECT_STRUCT_FIELD_ARRAY(HCollisionShapeDesc, ChildShapes, "")
CREFLECT_STRUCT_END()

CREFLECT_STRUCT_BEGIN(HCollisionSettings, "")
	CREFLECT_STRUCT_FIELD(bool,    Static, "")
	CREFLECT_STRUCT_FIELD(float,   Mass, "")
	CREFLECT_STRUCT_FIELD(float,   Restitution, "SliderMin(0) SliderMax(2)")
	CREFLECT_STRUCT_FIELD(float,   Friction,    "SliderMin(0) SliderMax(2)")
	CREFLECT_STRUCT_FIELD(float,   RollingFriction, "SliderMin(0) SliderMax(1)")
	CREFLECT_STRUCT_FIELD(float,   AngularDamping,  "SliderMin(0) SliderMax(1)")
	CREFLECT_STRUCT_FIELD(float,   AngularTreshold, "SliderMin(0) SliderMax(5)")
	CREFLECT_STRUCT_FIELD(Vector3, AngularFactor, "ColourChannels")
	CREFLECT_STRUCT_FIELD(float,   LinearTreshold, "SliderMin(0) SliderMax(5)")
	CREFLECT_STRUCT_FIELD(float,   LinearDamping,  "SliderMin(0) SliderMax(1)")
	CREFLECT_STRUCT_FIELD(Vector3, LinearFactor, "ColourChannels")
	CREFLECT_STRUCT_FIELD(HCollisionShapeDesc, Shape, "")
CREFLECT_STRUCT_END()
