#include <Editor/Inspector/ComponentEditorRigidbody.h>

namespace Columbus
{
	IMPLEMENT_COMPONENT_EDITOR(ComponentRigidbody, ComponentEditorRigidbody);

	int _RigidbodyEditId = 0;

	#define EDIT_TEMPLATE_BEGIN(x) \
		auto shape = dynamic_cast<x*>(Shape); \
		if (shape != nullptr) { \
			auto margin = Shape->GetMargin(); \
			ImGui::DragFloat(("Shape margin##_RigidbodyShapeEdit" + std::to_string(_RigidbodyEditId)).c_str(), &margin, 0.01f, 0, FLT_MAX); \
			if (margin != Shape->GetMargin()) \
				Shape->SetMargin(Math::Clamp(margin, 0.0f, FLT_MAX));

	#define EDIT_TEMPLATE_RESET(x) \
		float cached_margin = shape->GetMargin(); \
		RB->SetCollisionShape(nullptr); \
		if (_RigidbodyEditId == 0) \
			delete Shape; \
		shape = new x; \
		shape->SetMargin(cached_margin); \
		if (_RigidbodyEditId == 0) \
			RB->SetCollisionShape(shape); \
		return shape;

	#define EDIT_TEMPLATE_END() } return Shape;

	void ComponentEditorRigidbody::_EditShape(GameObject* Object, Rigidbody* RB, PhysicsShape*& Shape, int id)
	{
		_RigidbodyEditId = id;

		struct ShapeType
		{
			int id;
			const char* string;
			bool chosen;
			std::function<PhysicsShape * (const Transform&)> add;
			std::function<PhysicsShape * (GameObject*, Rigidbody*, PhysicsShape*&)> edit;
		};

		#define IS(x) dynamic_cast<x*>(Shape) != nullptr
		#define ADD(x) [&](const Transform& tran) { return new x; }

		Vector3 basePos(0, 0, 0);
		float baseRadius = 0.5f;

		ShapeType ShapeTypes[] =
		{
			{ 0, "Box", IS(PhysicsShapeBox), ADD(PhysicsShapeBox(tran.Scale)), _EditBox },
			{ 1, "Capsule", IS(PhysicsShapeCapsule), ADD(PhysicsShapeCapsule(Math::Max(tran.Scale.X, tran.Scale.Z), tran.Scale.Y)), _EditCapsule },
			{ 2, "Compound", IS(PhysicsShapeCompound), ADD(PhysicsShapeCompound()), _EditCompound },
			{ 3, "Cone", IS(PhysicsShapeCone), ADD(PhysicsShapeCone(Math::Max(tran.Scale.X, tran.Scale.Z), tran.Scale.Y)), _EditCone },
			//{ 4, "Convex hull", IS(PhysicsShapeConvexHull), ADD(PhysicsShapeConvexHull({})) },
			{ 4, "Cylinder", IS(PhysicsShapeCylinder), ADD(PhysicsShapeCylinder(tran.Scale)), _EditCylinder },
			{ 5, "Multi sphere", IS(PhysicsShapeMultiSphere), ADD(PhysicsShapeMultiSphere(&basePos, &baseRadius, 1)), _EditMultiSphere },
			{ 6, "Sphere", IS(PhysicsShapeSphere), ADD(PhysicsShapeSphere(Math::Max(Math::Max(tran.Scale.X, tran.Scale.Y), tran.Scale.Z))), _EditSphere },
		};

		int chosen = -1;
		for (auto& type : ShapeTypes)
		{
			if (type.chosen) chosen = type.id;
		}

		if (ImGui::BeginCombo(("Shape type##" + std::to_string(_RigidbodyEditId)).c_str(), chosen == -1 ? "None" : ShapeTypes[chosen].string))
		{
			for (auto& type : ShapeTypes)
			{
				if (ImGui::Selectable(type.string, type.chosen))
				{
					if (_RigidbodyEditId == 0)
						RB->SetCollisionShape(type.add(Object->transform));
					else
						Shape = type.add(Object->transform);

					chosen = type.id;
				}

				if (type.chosen)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		if (chosen != -1)
		{
			Shape = ShapeTypes[chosen].edit(Object, RB, Shape);
		}
	}

	PhysicsShape* ComponentEditorRigidbody::_EditBox(GameObject* Object, Rigidbody* RB, PhysicsShape*& Shape)
	{
		EDIT_TEMPLATE_BEGIN(PhysicsShapeBox)
			auto size = shape->GetSize();
		ImGui::DragFloat3(("Box size##_EditBox" + std::to_string(_RigidbodyEditId)).c_str(), (float*)&size, 0.1f, 0, FLT_MAX);

		if (size != shape->GetSize())
		{
			EDIT_TEMPLATE_RESET(PhysicsShapeBox(size.Clamp({ 0 }, { FLT_MAX })));
		}
		EDIT_TEMPLATE_END()
	}

	PhysicsShape* ComponentEditorRigidbody::_EditCapsule(GameObject* Object, Rigidbody* RB, PhysicsShape*& Shape)
	{
		EDIT_TEMPLATE_BEGIN(PhysicsShapeCapsule)
			auto radius = shape->GetRadius();
		auto height = shape->GetHeight();

		ImGui::DragFloat(("Capsule radius##_EditCapsule" + std::to_string(_RigidbodyEditId)).c_str(), &radius, 0.1f);
		ImGui::DragFloat(("Capsule height##_EditCapsule" + std::to_string(_RigidbodyEditId)).c_str(), &height, 0.1f);

		if (radius != shape->GetRadius() || height != shape->GetHeight())
		{
			EDIT_TEMPLATE_RESET(PhysicsShapeCapsule(Math::Clamp(radius, 0.0f, FLT_MAX), Math::Clamp(height, 0.0f, FLT_MAX)));
		}
		EDIT_TEMPLATE_END()
	}

	PhysicsShape* ComponentEditorRigidbody::_EditCompound(GameObject* Object, Rigidbody* RB, PhysicsShape*& Shape)
	{
		EDIT_TEMPLATE_BEGIN(PhysicsShapeCompound)
			for (int i = 0; i < shape->GetCount(); i++)
			{
				if (ImGui::CollapsingHeader((std::to_string(i) + "##_EditCompound_" + std::to_string(_RigidbodyEditId)).c_str()))
				{
					auto cachedShape = shape->GetShape(i);
					_EditShape(Object, RB, cachedShape, ++_RigidbodyEditId);

					if (cachedShape != shape->GetShape(i))
					{
						shape->UpdateChild(i, shape->GetTransform(i), cachedShape);
					}

					ImGui::Button("Remove");
				}
			}

		if (ImGui::Button(("Add##_EditCompound" + std::to_string(_RigidbodyEditId)).c_str()))
		{
			shape->AddChildShape(Transform(), new PhysicsShapeBox({ 1, 1, 1 }));
		}
		EDIT_TEMPLATE_END()
	}

	PhysicsShape* ComponentEditorRigidbody::_EditCone(GameObject* Object, Rigidbody* RB, PhysicsShape*& Shape)
	{
		EDIT_TEMPLATE_BEGIN(PhysicsShapeCone)
			auto radius = shape->GetRadius();
		auto height = shape->GetHeight();

		ImGui::DragFloat(("Cone radius##_EditCone" + std::to_string(_RigidbodyEditId)).c_str(), &radius, 0.1f);
		ImGui::DragFloat(("Cone height##_EditCone" + std::to_string(_RigidbodyEditId)).c_str(), &height, 0.1f);

		if (radius != shape->GetRadius() || height != shape->GetHeight())
		{
			EDIT_TEMPLATE_RESET(PhysicsShapeCone(Math::Clamp(radius, 0.0f, FLT_MAX), Math::Clamp(height, 0.0f, FLT_MAX)));
		}
		EDIT_TEMPLATE_END()
	}

	PhysicsShape* ComponentEditorRigidbody::_EditCylinder(GameObject* Object, Rigidbody* RB, PhysicsShape*& Shape)
	{
		EDIT_TEMPLATE_BEGIN(PhysicsShapeCylinder)
			auto size = shape->GetSize();
		ImGui::DragFloat3(("Cylinder size##_EditCylinder" + std::to_string(_RigidbodyEditId)).c_str(), (float*)&size, 0.1f);

		if (size != shape->GetSize())
		{
			EDIT_TEMPLATE_RESET(PhysicsShapeCylinder(size.Clamp({ 0 }, { FLT_MAX })));
		}
		EDIT_TEMPLATE_END()
	}

	PhysicsShape* ComponentEditorRigidbody::_EditMultiSphere(GameObject* Object, Rigidbody* RB, PhysicsShape*& Shape)
	{
		EDIT_TEMPLATE_BEGIN(PhysicsShapeMultiSphere)
			int changedId = -1;
		bool remove = false;
		Vector3 changePos{ 0,0,0 };
		float changeRadius = 0.5f;

		for (int i = 0; i < shape->GetCount(); i++)
		{
			if (ImGui::CollapsingHeader((std::to_string(i) + "##_EditMultiSphere_" + std::to_string(_RigidbodyEditId)).c_str()))
			{
				ImGui::Indent(10);
				auto pos = shape->GetPosition(i);
				auto radius = shape->GetRadius(i);

				ImGui::DragFloat3(("Position##_EditMultiSphere_" + std::to_string(i) + "_" + std::to_string(_RigidbodyEditId)).c_str(), (float*)&pos, 0.1f);
				ImGui::DragFloat(("Radius##_EditMultiSphere_" + std::to_string(i) + "_" + std::to_string(_RigidbodyEditId)).c_str(), &radius, 0.1f);

				if (shape->GetCount() > 1)
				{
					if (ImGui::Button(("Remove##_EditMultiSphere_" + std::to_string(i) + "_" + std::to_string(_RigidbodyEditId)).c_str()))
					{
						remove = true;
						changedId = i;
					}
				}

				if (pos != shape->GetPosition(i) || radius != shape->GetRadius(i))
				{
					remove = false;
					changedId = i;
					changePos = pos;
					changeRadius = Math::Clamp(radius, 0.0f, FLT_MAX);
				}

				ImGui::Unindent(10);
			}
		}

		if (ImGui::Button(("Add##_EditMultiSphere" + std::to_string(_RigidbodyEditId)).c_str()))
		{
			auto positions = shape->GetPositions();
			auto radiuses = shape->GetRadiuses();

			positions.emplace_back(0, 0, 0);
			radiuses.push_back(0.5f);

			EDIT_TEMPLATE_RESET(PhysicsShapeMultiSphere(positions.data(), radiuses.data(), positions.size()));
		}
		else if (changedId != -1)
		{
			auto positions = shape->GetPositions();
			auto radiuses = shape->GetRadiuses();

			if (remove)
			{
				positions.erase(positions.begin() + changedId);
				radiuses.erase(radiuses.begin() + changedId);
			}
			else
			{
				positions[changedId] = changePos;
				radiuses[changedId] = changeRadius;
			}

			EDIT_TEMPLATE_RESET(PhysicsShapeMultiSphere(positions.data(), radiuses.data(), positions.size()));
		}
		EDIT_TEMPLATE_END()
	}

	PhysicsShape* ComponentEditorRigidbody::_EditSphere(GameObject* Object, Rigidbody* RB, PhysicsShape*& Shape)
	{
		EDIT_TEMPLATE_BEGIN(PhysicsShapeSphere)
			auto radius = shape->GetRadius();
		ImGui::DragFloat(("Sphere radius##_EditSphere" + std::to_string(_RigidbodyEditId)).c_str(), &radius, 0.1f);

		if (radius != shape->GetRadius())
		{
			EDIT_TEMPLATE_RESET(PhysicsShapeSphere(Math::Clamp(radius, 0.0f, FLT_MAX)));
		}
		EDIT_TEMPLATE_END()
	}

}
