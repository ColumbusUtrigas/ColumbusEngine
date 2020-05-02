#include <Editor/Inspector/ComponentEditor.h>
#include <Scene/ComponentRigidbody.h>
#include <imgui/imgui.h>
#include <Physics/PhysicsShapeBox.h>
#include <Physics/PhysicsShapeCapsule.h>
#include <Physics/PhysicsShapeCompound.h>
#include <Physics/PhysicsShapeCone.h>
#include <Physics/PhysicsShapeConvexHull.h>
#include <Physics/PhysicsShapeCylinder.h>
#include <Physics/PhysicsShapeMultiSphere.h>
#include <Physics/PhysicsShapeSphere.h>

namespace Columbus
{

	class ComponentEditorRigidbody : public ComponentEditor
	{
		DECLARE_COMPONENT_EDITOR(ComponentRigidbody, ComponentEditorRigidbody);

	private:
		#define EDIT_TEMPLATE_BEGIN(x) \
			auto shape = dynamic_cast<x*>(Shape); \
			if (shape != nullptr) { \
				auto margin = Shape->GetMargin(); \
				ImGui::DragFloat("Shape margin", &margin, 0.01f, 0, FLT_MAX); \
				if (margin != Shape->GetMargin()) \
					Shape->SetMargin(Math::Clamp(margin, 0.0f, FLT_MAX));

		#define EDIT_TEMPLATE_RESET(x) \
			RB->SetCollisionShape(nullptr); \
			delete Shape; \
			shape = new x; \
			RB->SetCollisionShape(shape); \
			return shape;

		#define EDIT_TEMPLATE_END() } return Shape;

		static PhysicsShape* _EditBox(Rigidbody* RB, PhysicsShape* Shape)
		{
			EDIT_TEMPLATE_BEGIN(PhysicsShapeBox)
				auto size = shape->GetSize();
				ImGui::DragFloat3("Box size", (float*)&size, 0.1f);

				if (size != shape->GetSize())
				{
					EDIT_TEMPLATE_RESET(PhysicsShapeBox(size.Clamp({ 0 }, { FLT_MAX })));
				}
			EDIT_TEMPLATE_END()
		}

		static PhysicsShape* _EditCapsule(Rigidbody* RB, PhysicsShape* Shape)
		{
			EDIT_TEMPLATE_BEGIN(PhysicsShapeCapsule)
				auto radius = shape->GetRadius();
				auto height = shape->GetHeight();

				ImGui::DragFloat("Capsule radius", &radius, 0.1f);
				ImGui::DragFloat("Capsule height", &height, 0.1f);

				if (radius != shape->GetRadius() || height != shape->GetHeight())
				{
					EDIT_TEMPLATE_RESET(PhysicsShapeCapsule(Math::Clamp(radius, 0.0f, FLT_MAX), Math::Clamp(height, 0.0f, FLT_MAX)));
				}
			EDIT_TEMPLATE_END()
		}

		static PhysicsShape* _EditCone(Rigidbody* RB, PhysicsShape* Shape)
		{
			EDIT_TEMPLATE_BEGIN(PhysicsShapeCone)
				auto radius = shape->GetRadius();
				auto height = shape->GetHeight();

				ImGui::DragFloat("Cone radius", &radius, 0.1f);
				ImGui::DragFloat("Cone height", &height, 0.1f);

				if (radius != shape->GetRadius() || height != shape->GetHeight())
				{
					EDIT_TEMPLATE_RESET(PhysicsShapeCone(Math::Clamp(radius, 0.0f, FLT_MAX), Math::Clamp(height, 0.0f, FLT_MAX)));
				}
			EDIT_TEMPLATE_END()
		}

		static PhysicsShape* _EditCylinder(Rigidbody* RB, PhysicsShape* Shape)
		{
			EDIT_TEMPLATE_BEGIN(PhysicsShapeCylinder)
				auto size = shape->GetSize();
				ImGui::DragFloat3("Cylinder size", (float*)&size, 0.1f);

				if (size != shape->GetSize())
				{
					EDIT_TEMPLATE_RESET(PhysicsShapeCylinder(size.Clamp({ 0 }, { FLT_MAX })));
				}
			EDIT_TEMPLATE_END()
		}

		static PhysicsShape* _EditSphere(Rigidbody* RB, PhysicsShape* Shape)
		{
			EDIT_TEMPLATE_BEGIN(PhysicsShapeSphere)
				auto radius = shape->GetRadius();
				ImGui::DragFloat("Sphere radius", &radius, 0.1f);

				if (radius != shape->GetRadius())
				{
					EDIT_TEMPLATE_RESET(PhysicsShapeSphere(Math::Clamp(radius, 0.0f, FLT_MAX)));
				}
			EDIT_TEMPLATE_END()
		}
	public:
		void OnInspectorGUI() final override
		{
			auto RB = static_cast<ComponentRigidbody*>(Target)->GetRigidbody();
			if (RB != nullptr)
			{
				ImGui::Indent(10.0f);

				bool Static           = RB->IsStatic();
				float Mass            = RB->GetMass();
				float Restitution     = RB->GetRestitution();
				float Friction        = RB->GetFriction();
				float RollingFriction = RB->GetRollingFriction();
				float AngularDamping  = RB->GetAngularDamping();
				float AngularTreshold = RB->GetAngularTreshold();
				Vector3 AngularFactor = RB->GetAngularFactor();
				float LinearDamping   = RB->GetLinearDamping();
				float LinearTreshold  = RB->GetLinearTreshold();
				Vector3 LinearFactor  = RB->GetLinearFactor();
				Vector3 Gravity       = RB->GetGravity();
				PhysicsShape* Shape   = RB->GetCollisionShape();

				ImGui::Checkbox("Static##PanelInspector_Rigidbody", &Static);
				ImGui::DragFloat("Mass##PanelInspector_Rigidbody", &Mass, 0.1f, 0.001f, FLT_MAX);
				ImGui::DragFloat("Restitution##PanelInspector_Rigidbody", &Restitution, 0.1f, 0.0f, FLT_MAX);
				ImGui::DragFloat("Friction##PanelInspector_Rigidbody", &Friction, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Rolling Friction##PanelInspector_Rigidbody", &RollingFriction, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Angular Damping##PanelInspector_Rigidbody", &AngularDamping, 0.1f, 0.0f, FLT_MAX);
				ImGui::DragFloat("Angular Treshold##PanelInspector_Rigidbody", &AngularTreshold, 0.1f, 0.0f, FLT_MAX);
				ImGui::DragFloat3("Angular Factor##PanelInspector_Rigidbody", (float*)&AngularFactor, 0.02f, 0.0f, 1.0f);
				ImGui::DragFloat("Linear Damping##PanelInspector_Rigidbody", &LinearDamping, 0.1f, 0.0f, FLT_MAX);
				ImGui::DragFloat("Linear Treshold##PanelInspector_Rigidbody", &LinearTreshold, 0.1f, 0.0f, FLT_MAX);
				ImGui::DragFloat3("Linear Factor##PanelInspector_Rigidbody", (float*)&LinearFactor, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat3("Gravity##PanelInspector_Rigidbody", (float*)&Gravity, 0.1f);

				if (Static          != RB->IsStatic())           RB->SetStatic(Static);
				if (Mass            != RB->GetMass())            RB->SetMass(Mass);
				if (Restitution     != RB->GetRestitution())     RB->SetRestitution(Restitution);
				if (Friction        != RB->GetFriction())        RB->SetFriction(Friction);
				if (RollingFriction != RB->GetRollingFriction()) RB->SetRollingFriction(RollingFriction);
				if (AngularDamping  != RB->GetAngularDamping())  RB->SetAngularDamping(AngularDamping);
				if (AngularTreshold != RB->GetAngularTreshold()) RB->SetAngularTreshold(AngularTreshold);
				if (AngularFactor   != RB->GetAngularFactor())   RB->SetAngularFactor(AngularFactor);
				if (LinearDamping   != RB->GetLinearDamping())   RB->SetLinearDamping(LinearDamping);
				if (LinearTreshold  != RB->GetLinearTreshold())  RB->SetLinearTreshold(LinearTreshold);
				if (LinearFactor    != RB->GetLinearFactor())    RB->SetLinearFactor(LinearFactor);
				if (Gravity         != RB->GetGravity())         RB->SetGravity(Gravity);

				struct ShapeType
				{
					int id;
					const char* string;
					bool chosen;
					std::function<PhysicsShape*(const Transform&)> add;
					std::function<PhysicsShape*(Rigidbody*, PhysicsShape*)> edit;
				};

				#define IS(x) dynamic_cast<x*>(Shape) != nullptr
				#define ADD(x) [&](const Transform& tran) { return new x; }

				ShapeType ShapeTypes[] =
				{
					{ 0, "Box", IS(PhysicsShapeBox), ADD(PhysicsShapeBox(tran.Scale)), _EditBox },
					{ 1, "Capsule", IS(PhysicsShapeCapsule), ADD(PhysicsShapeCapsule(Math::Max(tran.Scale.X, tran.Scale.Z), tran.Scale.Y)), _EditCapsule },
					//compound
					{ 2, "Cone", IS(PhysicsShapeCone), ADD(PhysicsShapeCone(Math::Max(tran.Scale.X, tran.Scale.Z), tran.Scale.Y)), _EditCone },
					{ 3, "Convex hull", IS(PhysicsShapeConvexHull), ADD(PhysicsShapeConvexHull({})) },
					{ 4, "Cylinder", IS(PhysicsShapeCylinder), ADD(PhysicsShapeCylinder(tran.Scale)), _EditCylinder },
					// multi sphere
					{ 5, "Sphere", IS(PhysicsShapeSphere), ADD(PhysicsShapeSphere(Math::Max(Math::Max(tran.Scale.X, tran.Scale.Y), tran.Scale.Z))), _EditSphere },
				};

				int chosen = -1;
				for (auto& type : ShapeTypes)
				{
					if (type.chosen) chosen = type.id;
				}

				if (ImGui::BeginCombo("Shape type", chosen == -1 ? "None" : ShapeTypes[chosen].string))
				{
					for (auto& type : ShapeTypes)
					{
						if (ImGui::Selectable(type.string, type.chosen))
						{
							Log::Message("Selected %s", type.string);
							RB->SetCollisionShape(type.add(Target->gameObject->transform));
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
					Shape = ShapeTypes[chosen].edit(RB, Shape);
				}

				/*switch (Shape->mShape->getShapeType())
				{
				case BOX_SHAPE_PROXYTYPE: printf("BOX SHAPE\n"); break;
				}*/

				ImGui::Unindent(10.0f);
			}

			ImGui::Separator();
		}
	};
	IMPLEMENT_COMPONENT_EDITOR(ComponentRigidbody, ComponentEditorRigidbody);

}
