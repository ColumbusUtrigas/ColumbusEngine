#include <Editor/Inspector/ComponentEditor.h>
#include <Editor/Icons.h>
#include <Editor/CommonUI.h>
#include <Scene/ComponentRigidbody.h>
#include <Scene/GameObject.h>
#include <Physics/PhysicsShapeBox.h>
#include <Physics/PhysicsShapeCapsule.h>
#include <Physics/PhysicsShapeCompound.h>
#include <Physics/PhysicsShapeCone.h>
#include <Physics/PhysicsShapeConvexHull.h>
#include <Physics/PhysicsShapeCylinder.h>
#include <Physics/PhysicsShapeMultiSphere.h>
#include <Physics/PhysicsShapeSphere.h>
#include <imgui/imgui.h>
#include <functional>

namespace Columbus::Editor
{
	class ComponentEditorRigidbody : public ComponentEditor
	{
		DECLARE_COMPONENT_EDITOR(ComponentRigidbody, ComponentEditorRigidbody);
	public:
		void OnInspectorGUI() final override;
	};
	IMPLEMENT_COMPONENT_EDITOR(ComponentRigidbody, ComponentEditorRigidbody);

	static PhysicsShape* DrawShapesEditor(GameObject* Object, PhysicsShape* Shape, bool IsChild);

	static PhysicsShape* NewShape(PhysicsShape* OldShape, PhysicsShape* NewShape)
	{
		if (OldShape != nullptr)
		{
			NewShape->SetMargin(OldShape->GetMargin());
		}

		return NewShape;
	}

	static PhysicsShape* DrawShapeEditor(std::function<PhysicsShape * (GameObject * , PhysicsShape*)> func, const char* Name, GameObject* Object, PhysicsShape* Shape)
	{
		auto Margin = Shape->GetMargin();

		ImGui::PushID(Name);
		if (ImGui::DragFloat("Margin", &Margin, 0.01f, 0, FLT_MAX))
		{
			Margin = Math::Clamp(Margin, 0.0f, FLT_MAX);
			Shape->SetMargin(Margin);
		}
		Shape = func(Object, Shape);
		ImGui::PopID();

		return Shape;
	}

	static PhysicsShape* EditBox(GameObject* Object, PhysicsShape* Shape)
	{
		auto BoxShape = static_cast<PhysicsShapeBox*>(Shape);
		auto Size = BoxShape->GetSize();

		ImGui::DragFloat3("Size", (float*)&Size, 0.1f, 0, FLT_MAX);

		if (Size != BoxShape->GetSize())
		{
			Size.Clamp({ 0 }, { FLT_MAX });
			Shape = NewShape(Shape, new PhysicsShapeBox(Size));
		}

		return Shape;
	}

	static PhysicsShape* EditCapsule(GameObject* Object, PhysicsShape* Shape)
	{
		auto CapsuleShape = static_cast<PhysicsShapeCapsule*>(Shape);
		auto Radius = CapsuleShape->GetRadius();
		auto Height = CapsuleShape->GetHeight();

		ImGui::DragFloat("Radius", &Radius, 0.1f, 0, FLT_MAX);
		ImGui::DragFloat("Height", &Height, 0.1f, 0, FLT_MAX);

		if (Radius != CapsuleShape->GetRadius() || Height != CapsuleShape->GetHeight())
		{
			Radius = Math::Clamp(Radius, 0.0f, FLT_MAX);
			Height = Math::Clamp(Height, 0.0f, FLT_MAX);
			Shape = NewShape(Shape, new PhysicsShapeCapsule(Radius, Height));
		}

		return Shape;
	}

	static PhysicsShape* EditCompound(GameObject* Object, PhysicsShape* Shape)
	{
		auto CompoundShape = static_cast<PhysicsShapeCompound*>(Shape);

		char buf[64];

		if (ImGui::Button(ADD_ICON))
		{
			CompoundShape->AddChildShape(Transform(), new PhysicsShapeBox(Vector3{ 1 }));
		}
		ShowTooltipDelayed(CommonUISettings.TooltipDelay, "Add new child shape");

		for (int i = 0; i < CompoundShape->GetCount(); i++)
		{
			ImGui::PushID(i);

			ImGui::AlignTextToFramePadding();
			bool Open = ImGui::TreeNodeSized(itoa(i + 1, buf, 10), {}, ImGuiTreeNodeFlags_SpanAvailWidth);

			ImGui::SameLine();
			if (ImGui::Button(DELETE_ICON))
			{
				CompoundShape->RemoveChild(i);
				Open = false;
			}

			if (Open)
			{
				ImGui::TreePush();
				auto NewShape = DrawShapesEditor(Object, CompoundShape->GetShape(i), true);
				if (NewShape != CompoundShape->GetShape(i))
				{
					CompoundShape->UpdateChild(i, CompoundShape->GetTransform(i), NewShape);
				}
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
		
		return Shape;
	}

	static PhysicsShape* EditCone(GameObject* Object, PhysicsShape* Shape)
	{
		auto ConeShape = static_cast<PhysicsShapeCone*>(Shape);
		auto Radius = ConeShape->GetRadius();
		auto Height = ConeShape->GetHeight();

		ImGui::DragFloat("Radius", &Radius, 0.1f, 0, FLT_MAX);
		ImGui::DragFloat("Height", &Height, 0.1f, 0, FLT_MAX);

		if (Radius != ConeShape->GetRadius() || Height != ConeShape->GetHeight())
		{
			Radius = Math::Clamp(Radius, 0.0f, FLT_MAX);
			Height = Math::Clamp(Height, 0.0f, FLT_MAX);
			Shape = NewShape(Shape, new PhysicsShapeCone(Radius, Height));
		}

		return Shape;
	}

	static PhysicsShape* EditCylinder(GameObject* Object, PhysicsShape* Shape)
	{
		auto CylinderShape = static_cast<PhysicsShapeCylinder*>(Shape);
		auto Size = CylinderShape->GetSize();

		ImGui::DragFloat3("Size", (float*)&Size, 0.1f, 0, FLT_MAX);

		if (Size != CylinderShape->GetSize())
		{
			Size.Clamp({ 0 }, { FLT_MAX });
			Shape = NewShape(Shape, new PhysicsShapeCylinder(Size));
		}

		return Shape;
	}

	static PhysicsShape* EditMultiSphere(GameObject* Object, PhysicsShape* Shape)
	{
		auto MultiSphereShape = static_cast<PhysicsShapeMultiSphere*>(Shape);
		auto Positions = MultiSphereShape->GetPositions();
		auto Radiuses = MultiSphereShape->GetRadiuses();

		int ChangedID = -1;
		bool Remove = false;
		char buf[64];

		if (ImGui::Button(ADD_ICON))
		{
			Positions.emplace_back(0, 0, 0);
			Radiuses.push_back(0.5f);
			ChangedID = 0;
		}
		ShowTooltipDelayed(CommonUISettings.TooltipDelay, "Add new sphere");

		for (int i = 0; i < Positions.size(); i++)
		{
			ImGui::PushID(i);

			ImGui::AlignTextToFramePadding();
			bool Open = ImGui::TreeNodeSized(itoa(i + 1, buf, 10), {}, ImGuiTreeNodeFlags_SpanAvailWidth);

			if (Positions.size() > 1)
			{
				ImGui::SameLine();
				if (ImGui::Button(DELETE_ICON))
				{
					ChangedID = i;
					Remove = true;
				}
			}

			if (Open)
			{
				ImGui::TreePush();
				auto Position = Positions[i];
				auto Radius = Radiuses[i];

				ImGui::DragFloat3("Position", (float*)&Position, 0.1f);
				ImGui::DragFloat("Radius", &Radius, 0.1f, 0, FLT_MAX);

				if (Position != Positions[i] || Radius != Radiuses[i])
				{
					Positions[i] = Position;
					Radiuses[i] = Math::Clamp(Radius, 0.0f, FLT_MAX);
					ChangedID = i;
				}
				ImGui::TreePop();
			}

			ImGui::PopID();
		}

		if (ChangedID != -1)
		{
			if (Remove)
			{
				Positions.erase(Positions.begin() + ChangedID);
				Radiuses.erase(Radiuses.begin() + ChangedID);
			}

			Shape = NewShape(Shape, new PhysicsShapeMultiSphere(Positions.data(), Radiuses.data(), Positions.size()));
		}

		return Shape;
	}

	static PhysicsShape* EditSphere(GameObject* Object, PhysicsShape* Shape)
	{
		auto SphereShape = static_cast<PhysicsShapeSphere*>(Shape);
		auto Radius = SphereShape->GetRadius();

		ImGui::DragFloat("Radius", &Radius, 0.1f, 0, FLT_MAX);

		if (Radius != SphereShape->GetRadius())
		{
			Radius = Math::Clamp(Radius, 0.0f, FLT_MAX);
			Shape = NewShape(Shape, new PhysicsShapeSphere(Radius));
		}

		return Shape;
	}

	PhysicsShape* DrawShapesEditor(GameObject* Object, PhysicsShape* Shape, bool IsChild)
	{
		struct ShapeType
		{
			int id;
			const char* string;
			bool chosen;
			std::function<PhysicsShape * (const Transform&)> add;
			std::function<PhysicsShape * (GameObject*, PhysicsShape*)> edit;
		};

		#define IS(x) dynamic_cast<x*>(Shape) != nullptr
		#define ADD(x) [&](const Transform& tran) { return new x; }

		Vector3 BasePos(0, 0, 0);
		float BaseRadius = 0.5f;

		ShapeType ShapeTypes[] =
		{
			{ 0, "Box", IS(PhysicsShapeBox), ADD(PhysicsShapeBox(tran.Scale)), EditBox },
			{ 1, "Capsule", IS(PhysicsShapeCapsule), ADD(PhysicsShapeCapsule(Math::Max(tran.Scale.X, tran.Scale.Z), tran.Scale.Y)), EditCapsule },
			{ 2, "Compound", IS(PhysicsShapeCompound), ADD(PhysicsShapeCompound()), EditCompound },
			{ 3, "Cone", IS(PhysicsShapeCone), ADD(PhysicsShapeCone(Math::Max(tran.Scale.X, tran.Scale.Z), tran.Scale.Y)), EditCone },
			//{ 4, "Convex hull", IS(PhysicsShapeConvexHull), ADD(PhysicsShapeConvexHull({})) },
			{ 4, "Cylinder", IS(PhysicsShapeCylinder), ADD(PhysicsShapeCylinder(tran.Scale)), EditCylinder },
			{ 5, "Multi sphere", IS(PhysicsShapeMultiSphere), ADD(PhysicsShapeMultiSphere(&BasePos, &BaseRadius, 1)), EditMultiSphere },
			{ 6, "Sphere", IS(PhysicsShapeSphere), ADD(PhysicsShapeSphere(Math::Max(Math::Max(tran.Scale.X, tran.Scale.Y), tran.Scale.Z))), EditSphere },
		};

		int Chosen = -1;
		for (auto& Type : ShapeTypes)
		{
			if (Type.chosen) Chosen = Type.id;
		}

		// Draw shape selector
		if (ImGui::BeginCombo("Shape type", Chosen != -1 ? ShapeTypes[Chosen].string : "None"))
		{
			for (auto& Type : ShapeTypes)
			{
				if (ImGui::Selectable(Type.string, Type.chosen))
				{
					Shape = NewShape(Shape, Type.add(Object->transform));
					Chosen = Type.id;
				}

				if (Type.chosen)
				{
					ImGui::SetItemDefaultFocus();
				}
			}

			ImGui::EndCombo();
		}

		// Draw shape editor
		if (Chosen != -1)
		{
			Shape = DrawShapeEditor(ShapeTypes[Chosen].edit, ShapeTypes[Chosen].string, Object, Shape);
		}

		return Shape;
	}

	void ComponentEditorRigidbody::OnInspectorGUI()
	{
		auto RB = static_cast<ComponentRigidbody*>(Target)->GetRigidbody();
		if (RB != nullptr)
		{
			bool Static = RB->IsStatic();
			float Mass = RB->GetMass();
			float Restitution = RB->GetRestitution();
			float Friction = RB->GetFriction();
			float RollingFriction = RB->GetRollingFriction();
			float AngularDamping = RB->GetAngularDamping();
			float AngularTreshold = RB->GetAngularTreshold();
			Vector3 AngularFactor = RB->GetAngularFactor();
			float LinearDamping = RB->GetLinearDamping();
			float LinearTreshold = RB->GetLinearTreshold();
			Vector3 LinearFactor = RB->GetLinearFactor();
			Vector3 Gravity = RB->GetGravity();
			PhysicsShape* Shape = RB->GetCollisionShape();

			ImGui::Checkbox("Static", &Static);
			ImGui::DragFloat("Mass", &Mass, 0.1f, 0.001f, FLT_MAX);
			ImGui::DragFloat("Restitution", &Restitution, 0.1f, 0.0f, FLT_MAX);
			ImGui::DragFloat("Friction", &Friction, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Rolling Friction", &RollingFriction, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Angular Damping", &AngularDamping, 0.1f, 0.0f, FLT_MAX);
			ImGui::DragFloat("Angular Treshold", &AngularTreshold, 0.1f, 0.0f, FLT_MAX);
			ImGui::DragFloat3("Angular Factor", (float*)&AngularFactor, 0.02f, 0.0f, 1.0f);
			ImGui::DragFloat("Linear Damping", &LinearDamping, 0.1f, 0.0f, FLT_MAX);
			ImGui::DragFloat("Linear Treshold", &LinearTreshold, 0.1f, 0.0f, FLT_MAX);
			ImGui::DragFloat3("Linear Factor", (float*)&LinearFactor, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat3("Gravity", (float*)&Gravity, 0.1f);

			if (Static != RB->IsStatic())           RB->SetStatic(Static);
			if (Mass != RB->GetMass())            RB->SetMass(Mass);
			if (Restitution != RB->GetRestitution())     RB->SetRestitution(Restitution);
			if (Friction != RB->GetFriction())        RB->SetFriction(Friction);
			if (RollingFriction != RB->GetRollingFriction()) RB->SetRollingFriction(RollingFriction);
			if (AngularDamping != RB->GetAngularDamping())  RB->SetAngularDamping(AngularDamping);
			if (AngularTreshold != RB->GetAngularTreshold()) RB->SetAngularTreshold(AngularTreshold);
			if (AngularFactor != RB->GetAngularFactor())   RB->SetAngularFactor(AngularFactor);
			if (LinearDamping != RB->GetLinearDamping())   RB->SetLinearDamping(LinearDamping);
			if (LinearTreshold != RB->GetLinearTreshold())  RB->SetLinearTreshold(LinearTreshold);
			if (LinearFactor != RB->GetLinearFactor())    RB->SetLinearFactor(LinearFactor);
			if (Gravity != RB->GetGravity())         RB->SetGravity(Gravity);

			auto CachedShape = Shape;
			Shape = DrawShapesEditor(Target->gameObject, Shape, false);

			if (CachedShape != Shape)
			{
				delete CachedShape;
				RB->SetCollisionShape(Shape);
			}

			//_EditShape(Target->gameObject, RB, Shape, 0);
		}
	}

}
