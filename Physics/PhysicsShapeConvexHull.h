#pragma once

#include <Physics/PhysicsShape.h>
#include <Common/Model/Model.h>
#include <Core/Types.h>

#include <BulletCollision/CollisionShapes/btShapeHull.h>

namespace Columbus
{

	class PhysicsShapeConvexHull : public PhysicsShape
	{
	private:
		uint32 Count; //Count of points
		DECLARE_PROTOTYPE(PhysicsShape, PhysicsShapeConvexHull, "PhysicsShapeConvexHull", nullptr, 0)
	public:
		PhysicsShapeConvexHull(float* Points, uint32 InCount, bool OptimiseHull = true) :
			Count(0)
		{
			assert(InCount > 3);

			if (OptimiseHull)
			{
				// https://www.gamedev.net/forums/topic/691208-build-a-convex-hull-from-a-given-mesh-in-bullet/

				// needed to optimise convex hull
				btConvexHullShape TmpHull(Points, InCount, sizeof(float) * 3);

				//create a hull approximation
				TmpHull.setMargin(0);  // this is to compensate for a bug in bullet

				// optimisation procedure
				btShapeHull* Hull = new btShapeHull(&TmpHull);
				Hull->buildHull(0);    // note: parameter is ignored by buildHull

				mShape = new btConvexHullShape((const btScalar*)Hull->getVertexPointer(), Hull->numVertices(), sizeof(btVector3));
				this->Count = Hull->numVertices();

				delete Hull;
			}
			else
			{
				mShape = new btConvexHullShape(Points, InCount, sizeof(float) * 3);
				this->Count = InCount;
			}
		}

		PhysicsShapeConvexHull(const std::vector<Vector3>& Vertices, bool OptimiseHull = true) :
			PhysicsShapeConvexHull((float*)Vertices.data(), Vertices.size(), OptimiseHull)
		{
		}
		/*
		* Returns count of Convex-mesh points
		* @return uint32: Count of points
		*/
		uint32 GetCount() const
		{
			return Count;
		}

		PhysicsShape* Clone() const override
		{
			auto shape = new PhysicsShapeConvexHull(nullptr, 0);
			shape->SetMargin(Margin);
			return shape;
		}

		void Serialize(JSON& J) const override;
		void Deserialize(JSON& J) override;

		~PhysicsShapeConvexHull() { }
	};

}
