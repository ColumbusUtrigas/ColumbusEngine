#include <Editor/PanelInspector.h>
#include <Editor/FontAwesome.h>
#include <Editor/ResourcesViewerTexture.h>
#include <Editor/ResourcesViewerShader.h>
#include <Editor/ResourcesViewerMesh.h>
#include <Scene/ComponentAudioSource.h>
#include <Scene/ComponentLight.h>
#include <Scene/ComponentMeshRenderer.h>
#include <Scene/ComponentParticleSystem.h>
#include <Scene/ComponentRigidbody.h>
#include <Graphics/Device.h>
#include <Graphics/OpenGL/TextureOpenGL.h>
#include <Lib/imgui/imgui.h>
#include <Lib/imgui/misc/cpp/imgui_stdlib.h>
#include <climits>

// Icons for editor
#define TRANSFORM_ICON ICON_FA_ARROWS
#define MATERIAL_ICON ICON_FA_CIRCLE
#define AUDIO_ICON ICON_FA_MUSIC
#define LIGHT_ICON ICON_FA_LIGHTBULB_O
#define MESH_ICON ICON_FA_SPACE_SHUTTLE
#define PARTICLES_ICON ICON_FA_CERTIFICATE
#define RIGIDBODY_ICON ICON_FA_SQUARE

namespace Columbus
{

	static void DrawAddComponent(GameObject* GO);
	static void DrawTransformEditor(GameObject* GO);
	static void DrawMaterialEditor(GameObject* GO);
	static void DrawComponentsEditor(GameObject* GO);
	static void DrawComponentEditor(ComponentAudioSource* Co, GameObject& GO);
	static void DrawComponentEditor(ComponentLight* Co, GameObject& GO);
	static void DrawComponentEditor(ComponentMeshRenderer* Co, GameObject& GO);
	static void DrawComponentEditor(ComponentParticleSystem* Co, GameObject& GO);
	static void DrawComponentEditor(ComponentRigidbody* Co, GameObject& GO);



	void EditorPanelInspector::Draw()
	{
		if (Opened)
		{
			if (ImGui::Begin(ICON_FA_INFO" Inspector##PanelInspector", &Opened, ImGuiWindowFlags_NoCollapse))
			{
				if (Inspectable != nullptr)
				{
					ImGui::Checkbox("##PanelInspector_Enable", &Inspectable->Enable);
					ImGui::SameLine();
					ImGui::InputText("##PanelInspector_Name", &Inspectable->Name);
					ImGui::Separator();

					// Draw transform editor
					if (ImGui::CollapsingHeader(TRANSFORM_ICON" Transform##PanelInspector_Transform", ImGuiTreeNodeFlags_DefaultOpen))
					{
						DrawTransformEditor(Inspectable);
						ImGui::Separator();
					}

					//Draw material editor
					if (ImGui::CollapsingHeader(MATERIAL_ICON" Material##PanelInspector_Material"))
					{
						DrawMaterialEditor(Inspectable);
						ImGui::Separator();
					}

					// Draw components editor and "Add component" button
					DrawComponentsEditor(Inspectable);
					DrawAddComponent(Inspectable);
				}
			}
			ImGui::End();
		}
	}



	EditorPanelInspector::~EditorPanelInspector() {}



	void DrawAddComponent(GameObject* GO)
	{
		static int Selected = 0;
		static bool AddComponentEnable = false;

		ImGui::Dummy(ImVec2(0, 10));
		if (ImGui::Button("Add##PanelInspector_AddComponent", ImVec2(ImGui::GetWindowContentRegionWidth(), 0)))
		{
			AddComponentEnable = true;
		}

		const char* Name = "Add Component##PanelInspector_ModalWindow_AddComponent";
		const char* NameAudio = AUDIO_ICON" Audio Source##PanelInspector_ModalWindow_AddComponent_Audio";
		const char* NameLight = LIGHT_ICON" Light##PanelInspector_ModalWindow_AddComponent_Light";
		const char* NameMesh = MESH_ICON" Mesh Renderer##PanelInspector_ModalWindow_AddComponent_Mesh";
		const char* NameParticles = PARTICLES_ICON" Particle Emitter##PanelInspector_ModalWindow_AddComponent_Particles";
		const char* NameRigidbody = RIGIDBODY_ICON" Rigidbody##PanelInspector_ModalWindow_AddComponent_Rigidbody";


		// Flags for components' selectables and its size
		ImGuiSelectableFlags Flags = ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_DontClosePopups;
		ImVec2 Size = ImVec2(0, 50);

		auto Close = [&]()
		{
			AddComponentEnable = false;
			Selected = 0;
		};

		auto Add = [&]()
		{
			switch (Selected)
			{
			case 1: GO->AddComponent(new ComponentAudioSource(new AudioSource())); break;
			case 2: GO->AddComponent(new ComponentLight(new Light())); break;
			case 3: GO->AddComponent(new ComponentMeshRenderer(nullptr)); break;
			case 4: GO->AddComponent(new ComponentParticleSystem(ParticleEmitterCPU())); break;
			case 5: GO->AddComponent(new ComponentRigidbody(new Rigidbody())); break;
			default: break;
			}

			Close();
		};

		// Draw selectable for "Add component"
		auto Selectable = [&](const char* Name, int ID, bool Enable)
		{
			if (ImGui::Selectable(Name, Selected == ID, Flags | (Enable ? ImGuiSelectableFlags_Disabled : 0), Size))
			{
				Selected = ID;
				if (ImGui::IsMouseDoubleClicked(0)) Add();
			}
			ImGui::Separator();
		};

		bool IsAudio = GO->GetComponent<ComponentAudioSource>() != nullptr;
		bool IsLight = GO->GetComponent<ComponentLight>() != nullptr;
		bool IsMesh = GO->GetComponent<ComponentMeshRenderer>() != nullptr;
		bool IsParticles = GO->GetComponent<ComponentParticleSystem>() != nullptr;
		bool IsRigidbody = GO->GetComponent<ComponentRigidbody>() != nullptr;

		if (AddComponentEnable)
		{
			if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_Escape))) Close();

			ImGui::OpenPopup(Name);
			ImGui::SetNextWindowPosCenter(ImGuiCond_Always);
			ImGui::SetNextWindowSizeConstraints(ImVec2(450, 250), ImVec2(FLT_MAX, FLT_MAX));
			if (ImGui::BeginPopupModal(Name, &AddComponentEnable, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse))
			{
				ImVec2 size = ImVec2(ImGui::GetContentRegionMax().x, ImGui::GetContentRegionMax().y - 50.0f);
				if (ImGui::BeginChild("Components List##PanelInspector_ModalWindow_List", size))
				{
					Selectable(NameAudio, 1, IsAudio);
					Selectable(NameLight, 2, IsLight);
					Selectable(NameMesh, 3, IsMesh);
					Selectable(NameParticles, 4, IsParticles);
					Selectable(NameRigidbody, 5, IsRigidbody);
				}
				ImGui::EndChild();

				if (ImGui::BeginChild("Buttons##PanelInspector_ModalWindow_Buttons"))
				{
					if (ImGui::Button("Cancel##PanelInspector_ModalWindow_Cancel")) Close();
					ImGui::SameLine();
					if (ImGui::Button("Add##PanelInspector_ModalWindow_AddComponent_Add")) Add();
				}
				ImGui::EndChild();
				ImGui::EndPopup();
			}
		}
	}



	void DrawTransformEditor(GameObject* GO)
	{
		ImGui::Indent(10.0f);
		ImGui::DragFloat3("Position##Inspector_TransformEditor", (float*)&GO->transform.Position, 0.1f);
		ImGui::DragFloat3("Rotation##Inspector_TransformEditor", (float*)&GO->transform.Rotation, 0.1f);
		ImGui::DragFloat3("Scale##Inspector_TransformEditor",    (float*)&GO->transform.Scale,    0.1f);
		ImGui::Unindent(10.0f);
	}



	void DrawMaterialEditor(GameObject* GO)
	{
		const char* CullItems[] = { "No", "Front", "Back", "Front and back"};
		const char* DepthItems[] = { "Less", "Greater", "LEqual", "GEqual", "Equal", "Not equal", "Never", "Always" };

		ImGui::Indent(10.0f);

		ImGui::Combo("Culling##Inspector_MaterialEditor",    (int*)&GO->material.Culling,      CullItems,  4);
		ImGui::Combo("Depth test##Inspector_MaterialEditor", (int*)&GO->material.DepthTesting, DepthItems, 8);
		ImGui::Spacing();

		ImGui::Checkbox("Depth writing##Inspector_MaterialEditor", &GO->material.DepthWriting);
		ImGui::Checkbox("Transparent##Inspector_MaterialEditor",   &GO->material.Transparent);
		ImGui::Checkbox("Lighting##Inspector_MaterialEditor",      &GO->material.Lighting);
		ImGui::Spacing();
		
		ImGui::DragFloat2("Tiling##Inspector_MaterialEditor",        (float*)&GO->material.Tiling,           0.01f);
		ImGui::DragFloat2("Detail Tiling##Inspector_MaterialEditor", (float*)&GO->material.DetailTiling,     0.01f);
		ImGui::ColorEdit4("Albedo##Inspector_MaterialEditor",        (float*)&GO->material.Albedo);
		ImGui::SliderFloat("Roughness##Inspector_MaterialEditor",            &GO->material.Roughness,        0.00f, 1.0f);
		ImGui::SliderFloat("Metallic##Inspector_MaterialEditor",             &GO->material.Metallic,         0.00f, 1.0f);
		ImGui::DragFloat("Emission Strength##Inspector_MaterialEditor",      &GO->material.EmissionStrength, 0.01f);
		ImGui::Spacing();

		if (ImGui::Button("Shader##Inspector_MaterialEditor_Shader", ImVec2(ImGui::GetContentRegionAvail().x, 25)))
		{
			ResourcesViewerShader::Open(&GO->material.ShaderProg);
		}

		#define TEXID(a) a == nullptr ? 0 : (void*)(uintptr_t)(((TextureOpenGL*)(a))->GetID())
		#define TEXTURE_SELECTOR(a, text) \
			ImGui::PushID(text"##Inspector_MaterialEditor_Textures"); \
			if (ImGui::ImageButton(TEXID(a), TexSize)) \
				ResourcesViewerTexture::Open(&a); \
			ImGui::PopID(); \
			ImGui::SameLine(); \
			ImGui::Text(text);

		ImVec2 TexSize(30, 10);

		// This is a block of texture selectors, which are: ImageButton which activates
		// texture viewer and name of the texture
		TEXTURE_SELECTOR(GO->material.AlbedoMap,       "Albedo");
		TEXTURE_SELECTOR(GO->material.NormalMap,       "Normal");
		TEXTURE_SELECTOR(GO->material.RoughnessMap,    "Roughness");
		TEXTURE_SELECTOR(GO->material.MetallicMap,     "Metallic");
		TEXTURE_SELECTOR(GO->material.OcclusionMap,    "Occlusion");
		TEXTURE_SELECTOR(GO->material.EmissionMap,     "Emission");
		TEXTURE_SELECTOR(GO->material.DetailAlbedoMap, "Detail Albedo");
		TEXTURE_SELECTOR(GO->material.DetailNormalMap, "Detail Normal");

		#undef TEXTURE_SELECTOR
		#undef TEXID

		ImGui::Unindent(10.0f);
	}


	// Draw all of the component editors. Component editors are drawing
	// (if exist) in collapsing header.
	void DrawComponentsEditor(GameObject* GO)
	{
		DrawComponentEditor(GO->GetComponent<ComponentAudioSource>(), *GO);
		DrawComponentEditor(GO->GetComponent<ComponentLight>(), *GO);
		DrawComponentEditor(GO->GetComponent<ComponentMeshRenderer>(), *GO);
		DrawComponentEditor(GO->GetComponent<ComponentParticleSystem>(), *GO);
		DrawComponentEditor(GO->GetComponent<ComponentRigidbody>(), *GO);
	}



	void DrawComponentEditor(ComponentAudioSource* Co, GameObject& GO)
	{
		if (Co != nullptr)
		{
			bool Shown = ImGui::CollapsingHeader(AUDIO_ICON" Audio Source##PanelInspector_Audio");
			/*if (ImGui::IsItemHovered() && ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_Space)))
			{
				GO.DeleteComponent<ComponentAudioSource>();
				return;
			}*/

			if (Shown)
			{

				auto Source = Co->GetSource();
				if (Source != nullptr)
				{
					const char* Types[] = { "2D", "3D"};

					ImGui::Indent(10.0f);

					ImGui::Combo("Type##PanelInspector_Audio",       (int*)&Source->SoundMode, Types, 2);
					ImGui::DragFloat("Gain##PanelInspector_Audio",         &Source->Gain,        0.01f, 0.0f, 1.0f);
					ImGui::DragFloat("Pitch##PanelInspector_Audio",        &Source->Pitch,       0.01f, 0.0f, 1.0f);
					ImGui::DragFloat("Min Distance##PanelInspector_Audio", &Source->MinDistance, 0.1f,  0.1f, 1000.0f);
					ImGui::DragFloat("Max Distance##PanelInspector_Audio", &Source->MaxDistance, 0.1f,  1.0f, 1000.0f);
					ImGui::DragFloat("Rolloff##PanelInspector_Audio",      &Source->Rolloff,     0.1f,  0.0f, 10.0f);
					ImGui::Checkbox("Play##PanelInspector_Audio",          &Source->Playing);
					ImGui::Checkbox("Loop##PanelInspector_Audio",          &Source->Looping);

					ImGui::Unindent(10.0f);
				}

				ImGui::Separator();
			}
		}
	}



	void DrawComponentEditor(ComponentLight* Co, GameObject& GO)
	{
		if (Co != nullptr)
		{
			if (ImGui::CollapsingHeader(LIGHT_ICON" Light##PanelInspector_Light"))
			{
				auto Light = Co->GetLight();
				if (Light != nullptr)
				{
					const char* Types[] = { "Directional", "Point", "Spot"};

					ImGui::Indent(10.0f);

					ImGui::Combo("Type##PanelInspector_Light",               &Light->Type, Types, 3);
					ImGui::ColorEdit3("Color##PanelInspector_Light", (float*)&Light->Color);
					ImGui::DragFloat("Energy##PanelInspector_Light",         &Light->Energy, 0.1f, 0.0f, FLT_MAX);
					ImGui::DragFloat("Range##PanelInspector_Light",          &Light->Range,  0.1f);
					ImGui::DragFloat("Inner Cutoff##PanelInspector_Light",   &Light->InnerCutoff, 0.01f);
					ImGui::DragFloat("Outer Cutoff##PanelInspector_Light",   &Light->OuterCutoff, 0.01f);

					ImGui::Unindent(10.0f);
				}

				ImGui::Separator();
			}
		}
	}



	void DrawComponentEditor(ComponentMeshRenderer* Co, GameObject& GO)
	{
		if (Co != nullptr)
		{
			if (ImGui::CollapsingHeader(MESH_ICON" Mesh Renderer##PanelInspector"))
			{
				if (ImGui::Button("Mesh##PanelInspector_MeshRenderer", ImVec2(ImGui::GetContentRegionAvail().x, 25)))
				{
					ResourcesViewerMesh::Open(&Co->GetMesh());
				}
			}
		}
	}



	void DrawComponentEditor(ComponentParticleSystem* Co, GameObject& GO)
	{
		if (Co != nullptr)
		{
			if (ImGui::CollapsingHeader(PARTICLES_ICON" Particle System##PanelInspector"))
			{
				const char* BlendModes[] = { "Default", "Add", "Subtract", "Multiply" };
				const char* BillboardModes[] = { "None", "Vertical", "Horizontal", "Face to camera" };
				const char* TransformationModes[] = { "World", "Local" };
				const char* SortModes[] = { "None", "Young first", "Old first", "Nearest first" };

				#define NAME(a)    a"##PanelInspector_ParticleSystem"

				auto Emitter = &Co->GetEmitter();

				ImGui::Indent(10.0f);
				
				ImGui::DragInt  (NAME("Max Particles"), (int*)&Emitter->MaxParticles, 1, 0, 1024);
				ImGui::DragFloat(NAME("Emit Rate"), &Emitter->EmitRate, 0.1f, 0.0f, FLT_MAX);
				ImGui::Checkbox (NAME("Emit"),      &Emitter->Emit);
				ImGui::Checkbox (NAME("Visible"),   &Emitter->Visible);

				ImGui::Combo(NAME("Blend mode"),          (int*)&Emitter->Blend, BlendModes, 4);
				ImGui::Combo(NAME("Billboard mode"),      (int*)&Emitter->Billboard, BillboardModes, 4);
				ImGui::Combo(NAME("Transformation mode"), (int*)&Emitter->Transformation, TransformationModes, 2);
				ImGui::Combo(NAME("Sort mode"),           (int*)&Emitter->Sort, SortModes, 4);

				ImGui::Separator();

				if (ImGui::CollapsingHeader(NAME("Lifetime")))
				{
					ImGui::Indent(10.0f);
					ImGui::DragFloatRange2(NAME("Lifetime##Lifetime"), &Emitter->ModuleLifetime.Min, &Emitter->ModuleLifetime.Max, 0.1f);
					ImGui::Unindent(10.0f);
					ImGui::Separator();
				}

				if (ImGui::CollapsingHeader(NAME("Location")))
				{
					const char* Shapes[] = { "Point", "Box", "Circle", "Sphere" };

					ImGui::Indent(10.0f);
					ImGui::DragFloat3(NAME("Size##Location"),  (float*)&Emitter->ModuleLocation.Size, 0.1f, 0.0f, FLT_MAX);
					ImGui::DragFloat(NAME("Radius##Location"),         &Emitter->ModuleLocation.Radius, 0.1f, 0.0f, FLT_MAX);
					ImGui::Combo(NAME("Shape##Location"),        (int*)&Emitter->ModuleLocation.Shape, Shapes, 4);
					ImGui::Checkbox(NAME("Emit from shell##Location"), &Emitter->ModuleLocation.EmitFromShell);
					ImGui::Unindent(10.0f);
					ImGui::Separator();
				}

				if (ImGui::CollapsingHeader(NAME("Velocity")))
				{
					ImGui::Indent(10.0f);
					ImGui::DragFloat3(NAME("Min##Velocity"), (float*)&Emitter->ModuleVelocity.Min, 0.1f);
					ImGui::DragFloat3(NAME("Max##Velocity"), (float*)&Emitter->ModuleVelocity.Max, 0.1f);
					ImGui::Unindent(10.0f);
					ImGui::Separator();
				}

				if (ImGui::CollapsingHeader(NAME("Rotation")))
				{
					ImGui::Indent(10.0f);
					ImGui::DragFloatRange2("Initial##PanelInspector_ParticleSystem_Rotation", &Emitter->ModuleRotation.Min, &Emitter->ModuleRotation.Max, 0.1f);
					ImGui::DragFloatRange2("Velocity##PanelInspector_ParticleSystem_Rotation", &Emitter->ModuleRotation.MinVelocity, &Emitter->ModuleRotation.MaxVelocity, 0.1f);
					ImGui::Unindent(10.0f);
					ImGui::Separator();
				}

				if (ImGui::CollapsingHeader(NAME("Color")))
				{
					const char* Modes[] = { "Initial", "Over life" };

					ImGui::Indent(10.0f);
					
					ImGui::Combo(NAME("Mode##Color"), (int*)&Emitter->ModuleColor.Mode, Modes, 2);

					if (Emitter->ModuleColor.Mode == ParticleModuleColor::UpdateMode::Initial)
					{
						ImGui::DragFloat4(NAME("Min##Color"), (float*)&Emitter->ModuleColor.Min);
						ImGui::DragFloat4(NAME("Max##Color"), (float*)&Emitter->ModuleColor.Max);
					}

					if (Emitter->ModuleColor.Mode == ParticleModuleColor::UpdateMode::OverLife)
					{
						bool Editing = false;
						size_t Counter = 1;
						for (auto& Node : Emitter->ModuleColor.Curve.Points)
						{
							auto str = std::to_string(Counter);

							float Min = Counter == 1 ? 0.0f : Emitter->ModuleColor.Curve.Points[Counter - 2].first;
							float Max = Counter == Emitter->ModuleColor.Curve.Points.size() ? 1.0f : Emitter->ModuleColor.Curve.Points[Counter].first;

							if (ImGui::CollapsingHeader((str + NAME("##ColorOverLife_Point")).c_str()))
							{
								auto p = NAME("Value##ColorOverLife") + str;
								auto c = NAME("Color##ColorOverLife") + str;
								auto r = NAME("Remove##ColorOverLife") + str;

								ImGui::Indent(10.0f);
								ImGui::SetNextItemWidth(50.0f);

								ImGui::DragFloat(p.c_str(), &Node.first, 0.05f, Min, Max);
								if (ImGui::IsItemActive())
									Editing = true;

								ImGui::ColorEdit4(c.c_str(), (float*)&Node.second);

								if (ImGui::Button(r.c_str())) Emitter->ModuleColor.Curve.RemovePoint(Counter - 1);
								ImGui::Unindent(10.0f);
							}

							Counter++;
						}

						if (!Editing)
						{
							Emitter->ModuleColor.Curve.Sort();
						}

						if (ImGui::Button(NAME("Add##ColorOverLife")))
						{
							Emitter->ModuleColor.Curve.AddPoint(Vector4(1), 1.0f);
						}
					}

					ImGui::Unindent(10.0f);
					ImGui::Separator();
				}

				if (ImGui::CollapsingHeader(NAME("Size")))
				{
					const char* Modes[] = { "Initial", "Over life" };

					ImGui::Indent(10.0f);

					ImGui::Combo(NAME("Mode##Size"), (int*)&Emitter->ModuleSize.Mode, Modes, 2);

					if (Emitter->ModuleSize.Mode == ParticleModuleSize::UpdateMode::Initial)
					{
						ImGui::DragFloat3(NAME("Min##Size"), (float*)&Emitter->ModuleSize.Min);
						ImGui::DragFloat3(NAME("Max##Size"), (float*)&Emitter->ModuleSize.Max);
					}

					if (Emitter->ModuleSize.Mode == ParticleModuleSize::UpdateMode::OverLife)
					{
						bool Editing = false;
						size_t Counter = 1;
						for (auto& Node : Emitter->ModuleSize.Curve.Points)
						{
							auto str = std::to_string(Counter);

							float Min = Counter == 1 ? 0.0f : Emitter->ModuleSize.Curve.Points[Counter - 2].first;
							float Max = Counter == Emitter->ModuleSize.Curve.Points.size() ? 1.0f : Emitter->ModuleSize.Curve.Points[Counter].first;

							if (ImGui::CollapsingHeader((str + NAME("##SizeOverLife_Point")).c_str()))
							{
								auto p = NAME("Value##SizeOverLife") + str;
								auto c = NAME("Size##SizeOverLife") + str;
								auto r = NAME("Remove##SizeOverLife") + str;

								ImGui::Indent(10.0f);
								ImGui::SetNextItemWidth(50.0f);

								ImGui::DragFloat(p.c_str(), &Node.first, 0.05f, Min, Max);
								if (ImGui::IsItemActive())
									Editing = true;

								ImGui::DragFloat3(c.c_str(), (float*)&Node.second, 0.1f, 0.0f, FLT_MAX);
								if (ImGui::Button(r.c_str())) Emitter->ModuleSize.Curve.RemovePoint(Counter - 1);
								ImGui::Unindent(10.0f);
							}

							Counter++;
						}

						if (!Editing)
						{
							Emitter->ModuleSize.Curve.Sort();
						}

						if (ImGui::Button(NAME("Add##SizeOverLife")))
						{
							Emitter->ModuleSize.Curve.AddPoint(Vector3(1), 1.0f);
						}
					}

					ImGui::Unindent(10.0f);
					ImGui::Separator();
				}

				if (ImGui::CollapsingHeader(NAME("Noise")))
				{
					ImGui::Indent(10.0f);

					ImGui::DragFloat(NAME("Strength##Noise"),    &Emitter->ModuleNoise.Strength,    0.1f, 0.0f, FLT_MAX);
					ImGui::SliderInt(NAME("Octaves##Noise"),     &Emitter->ModuleNoise.Octaves,        1,    8);
					ImGui::DragFloat(NAME("Lacunarity##Noise"),  &Emitter->ModuleNoise.Lacunarity,  0.1f, 0.0f, FLT_MAX);
					ImGui::DragFloat(NAME("Persistence##Noise"), &Emitter->ModuleNoise.Persistence, 0.1f, 0.0f, FLT_MAX);
					ImGui::DragFloat(NAME("Frequency##Noise"),   &Emitter->ModuleNoise.Frequency,   0.1f, 0.0f, FLT_MAX);
					ImGui::DragFloat(NAME("Amplitude##Noise"),   &Emitter->ModuleNoise.Amplitude,   0.1f, 0.0f, FLT_MAX);

					ImGui::Unindent(10.0f);
				}

				if (ImGui::CollapsingHeader(NAME("SubUV")))
				{
					const char* Modes[] =  { "Linear", "Random" };

					ImGui::Indent(10.0f);

					ImGui::Combo(NAME("Mode##SubUV"),   (int*)&Emitter->ModuleSubUV.Mode, Modes, 2);
					ImGui::DragInt(NAME("Horizontal##SubUV"), &Emitter->ModuleSubUV.Horizontal, 1,    0, INT_MAX);
					ImGui::DragInt(NAME("Vertical##SubUV"),   &Emitter->ModuleSubUV.Vertical,   1,    0, INT_MAX);
					ImGui::DragFloat(NAME("Cycles##SubUV"),   &Emitter->ModuleSubUV.Cycles,  0.1f, 0.0f, FLT_MAX);

					ImGui::Unindent(10.0f);
				}

				ImGui::Unindent(10.0f);
				ImGui::Separator();

				#undef NAME
			}
		}
	}

	void DrawComponentEditor(ComponentRigidbody* Co, GameObject& GO)
	{
		if (Co != nullptr)
		{
			if (ImGui::CollapsingHeader(RIGIDBODY_ICON" Rigidbody##PanelInspector_Rigidbody"))
			{
				auto RB = Co->GetRigidbody();
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
		}
	}

}


