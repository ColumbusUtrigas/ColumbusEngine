#include <Physics/PhysicsShapeBox.h>
#include <Physics/PhysicsShapeCapsule.h>
#include <Physics/PhysicsShapeCompound.h>
#include <Physics/PhysicsShapeCone.h>
#include <Physics/PhysicsShapeConvexHull.h>
#include <Physics/PhysicsShapeCylinder.h>
#include <Physics/PhysicsShapeMultiSphere.h>
#include <Physics/PhysicsShapeSphere.h>
#include <Common/JSON/JSON.h>

namespace Columbus
{

	IMPLEMENT_PROTOTYPE(PhysicsShape, PhysicsShapeBox);
	IMPLEMENT_PROTOTYPE(PhysicsShape, PhysicsShapeCapsule);
	IMPLEMENT_PROTOTYPE(PhysicsShape, PhysicsShapeCompound);
	IMPLEMENT_PROTOTYPE(PhysicsShape, PhysicsShapeCone);
	IMPLEMENT_PROTOTYPE(PhysicsShape, PhysicsShapeConvexHull);
	IMPLEMENT_PROTOTYPE(PhysicsShape, PhysicsShapeCylinder);
	IMPLEMENT_PROTOTYPE(PhysicsShape, PhysicsShapeMultiSphere);
	IMPLEMENT_PROTOTYPE(PhysicsShape, PhysicsShapeSphere);

	void PhysicsShape::BaseSerialize(JSON& J) const
	{
		J["Margin"] = Margin;
		J["Type"] = std::string(GetTypename());
	}

	void PhysicsShape::BaseDeserialize(JSON& J)
	{
		Margin = J["Margin"].GetFloat();
	}

	void PhysicsShapeBox::Serialize(JSON& J) const
	{
		BaseSerialize(J);
		J["Size"] = Size;
	}

	void PhysicsShapeBox::Deserialize(JSON& J)
	{
		BaseDeserialize(J);
		Size = J["Size"].GetVector3<float>();
	}

	void PhysicsShapeCapsule::Serialize(JSON& J) const
	{
		BaseSerialize(J);
		J["Radius"] = Radius;
		J["Height"] = Height;
	}

	void PhysicsShapeCapsule::Deserialize(JSON& J)
	{
		BaseDeserialize(J);
		Radius = J["Radius"].GetFloat();
		Height = J["Height"].GetFloat();
	}

	void PhysicsShapeCompound::Serialize(JSON& J) const
	{
		BaseSerialize(J);
		for (int i = 0; i < Count; i++)
		{
			J["Children"][i]["LocalTransform"] = Transforms[i];
			J["Children"][i]["Shape"] = Shapes[i];
		}
	}

	void PhysicsShapeCompound::Deserialize(JSON& J)
	{
		BaseDeserialize(J);
	}

	void PhysicsShapeCone::Serialize(JSON& J) const
	{
		BaseSerialize(J);
		J["Radius"] = Radius;
		J["Height"] = Height;
	}

	void PhysicsShapeCone::Deserialize(JSON& J)
	{
		BaseDeserialize(J);
		Radius = J["Radius"].GetFloat();
		Height = J["Height"].GetFloat();
	}

	void PhysicsShapeConvexHull::Serialize(JSON& J) const
	{
		BaseSerialize(J);
	}

	void PhysicsShapeConvexHull::Deserialize(JSON& J)
	{
		BaseDeserialize(J);
	}

	void PhysicsShapeCylinder::Serialize(JSON& J) const
	{
		BaseSerialize(J);
		J["Size"] = Size;
	}

	void PhysicsShapeCylinder::Deserialize(JSON& J)
	{
		BaseDeserialize(J);
		Size = J["Size"].GetVector3<float>();
	}

	void PhysicsShapeMultiSphere::Serialize(JSON& J) const
	{
		BaseSerialize(J);
		for (int i = 0; i < Count; i++)
		{
			J["Spheres"][i]["Position"] = Positions[i];
			J["Spheres"][i]["Radius"] = Radiuses[i];
		}
	}

	void PhysicsShapeMultiSphere::Deserialize(JSON& J)
	{
		BaseDeserialize(J);
		if (J["Spheres"].IsArray())
		{
			Positions.clear();
			Radiuses.clear();
			Count = 0;

			for (int i = 0; i < J["Spheres"].GetElementsCount(); i++)
			{
				Positions.push_back(J["Spheres"][i]["Position"].GetVector3<float>());
				Radiuses.push_back(J["Spheres"][i]["Radius"].GetFloat());
				Count++;
			}
		}
	}

	void PhysicsShapeSphere::Serialize(JSON& J) const
	{
		BaseSerialize(J);
		J["Radius"] = Radius;
	}

	void PhysicsShapeSphere::Deserialize(JSON& J)
	{
		BaseDeserialize(J);
		Radius = J["Radius"].GetFloat();
	}
}
