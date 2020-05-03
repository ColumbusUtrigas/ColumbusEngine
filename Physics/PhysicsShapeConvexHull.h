#pragma once

#include <Physics/PhysicsShape.h>
#include <Common/Model/Model.h>
#include <Core/Types.h>

namespace Columbus
{

	class PhysicsShapeConvexHull : public PhysicsShape
	{
	private:
		uint32 Count; //Count of points
		DECLARE_PROTOTYPE(PhysicsShape, PhysicsShapeConvexHull, "PhysicsShapeConvexHull", nullptr, 0)
	public:
		PhysicsShapeConvexHull(float* Points, uint32 Count) :
			Count(0)
		{
			if (Points != nullptr && Count != 0)
			{
				mShape = new btConvexHullShape(Points, Count, sizeof(float) * 3);
				this->Count = Count;
			}
		}

		PhysicsShapeConvexHull(const std::vector<Vertex>& Vertices) :
			Count(0)
		{
			if (Vertices.size() != 0)
			{
				float* Points = new float[Vertices.size() * 3];
				uint64 Counter = 0;
				for (auto& Vertex : Vertices)
				{
					Points[Counter++] = Vertex.pos.X;
					Points[Counter++] = Vertex.pos.Y;
					Points[Counter++] = Vertex.pos.Z;
				}

				this->Count = Vertices.size();
				mShape = new btConvexHullShape(Points, Vertices.size(), sizeof(float) * 3);
				delete[] Points;
			}
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
