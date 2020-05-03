#pragma once

#include <Physics/PhysicsShape.h>
#include <Scene/Transform.h>

namespace Columbus
{

	class PhysicsShapeCompound : public PhysicsShape
	{
	protected:
		uint32 Count;
		std::vector<Transform> Transforms;
		std::vector<PhysicsShape*> Shapes;
		DECLARE_PROTOTYPE(PhysicsShape, PhysicsShapeCompound, "PhysicsShapeCompound")
	public:
		PhysicsShapeCompound() : Count(0)
		{
			mShape = new btCompoundShape();
		}
		/*
		* Add child collision shape
		* @param Transform LocalTransform: Local transform of child collision shape
		* @param PhysicsShape* Shape: New child collision shape
		*/
		void AddChildShape(const Transform& LocalTransform, PhysicsShape* Shape)
		{
			if (Shape != nullptr)
			{
				if (Shape->IsExist())
				{
					btTransform bTransform;
					const auto& Pos = LocalTransform.Position;
					const auto& Rot = LocalTransform.Rotation;

					bTransform.setOrigin(btVector3(Pos.X, Pos.Y, Pos.Z));
					bTransform.setRotation(btQuaternion(Rot.X, Rot.Y, Rot.Z, Rot.W));

					static_cast<btCompoundShape*>(mShape)->addChildShape(bTransform, Shape->mShape);

					Transforms.push_back(LocalTransform);
					Shapes.push_back(Shape);
					Count++;
				}
			}
		}

		void RemoveChild(uint32 Id)
		{
			if (Shapes.size() > Id)
			{
				auto shape = Shapes[Id]->mShape;
				static_cast<btCompoundShape*>(mShape)->removeChildShape(shape);

				Shapes.erase(Shapes.begin() + Id);
				Transforms.erase(Transforms.begin() + Id);
				Count--;
			}
		}

		void UpdateChild(uint32 Id, const Transform& LocalTransform, PhysicsShape* Shape)
		{
			if (Shapes.size() > Id)
			{
				auto shapes = Shapes;
				auto trans = Transforms;

				for (int i = Shapes.size(); i >= 0; i--)
				{
					RemoveChild(i);
				}

				shapes[Id] = Shape;
				trans[Id] = LocalTransform;

				for (int i = 0; i < shapes.size(); i++)
				{
					AddChildShape(trans[i], shapes[i]);
				}
			}
		}

		uint32 GetCount() const
		{
			return Count;
		}

		Transform GetTransform(uint32 Id) const
		{
			return Transforms[Id];
		}

		PhysicsShape* GetShape(uint32 Id) const
		{
			return Shapes[Id];
		}

		PhysicsShape* Clone() const override
		{
			auto shape = new PhysicsShapeCompound();
			for (int i = 0; i < Shapes.size(); i++)
			{
				shape->AddChildShape(Transforms[i], Shapes[i]);
			}
			shape->SetMargin(Margin);
			return shape;
		}

		void Serialize(JSON& J) const override;
		void Deserialize(JSON& J) override;

		~PhysicsShapeCompound() {}
	};

}
