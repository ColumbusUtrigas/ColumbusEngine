#include <Editor/Inspector/ComponentEditor.h>
#include <Scene/ComponentRigidbody.h>
#include <imgui/imgui.h>

namespace Columbus
{

	class ComponentEditorRigidbody : public ComponentEditor
	{
		DECLARE_COMPONENT_EDITOR(ComponentRigidbody, ComponentEditorRigidbody);
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
				ImGui::DragFloat3("Gravity##PanelInspector_Rigidbody", (float*)&Gravity, 0.1f, 0.0f, FLT_MAX);

				RB->SetStatic(Static);
				RB->SetMass(Mass);
				RB->SetRestitution(Restitution);
				RB->SetFriction(Friction);
				RB->SetRollingFriction(RollingFriction);
				RB->SetAngularDamping(AngularDamping);
				RB->SetAngularTreshold(AngularTreshold);
				RB->SetAngularFactor(AngularFactor);
				RB->SetLinearDamping(LinearDamping);
				RB->SetLinearTreshold(LinearTreshold);
				RB->SetLinearFactor(LinearFactor);
				RB->SetGravity(Gravity);

				ImGui::Unindent(10.0f);
			}

			ImGui::Separator();
		}
	};
	IMPLEMENT_COMPONENT_EDITOR(ComponentRigidbody, ComponentEditorRigidbody);

}
