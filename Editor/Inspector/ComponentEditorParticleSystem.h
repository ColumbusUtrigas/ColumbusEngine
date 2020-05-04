#include <Editor/Inspector/ComponentEditor.h>
#include <Scene/ComponentParticleSystem.h>
#include <imgui/imgui.h>

namespace Columbus
{

	class ComponentEditorParticleSystem : public ComponentEditor
	{
		DECLARE_COMPONENT_EDITOR(ComponentParticleSystem, ComponentEditorParticleSystem);
	public:
		void OnInspectorGUI() final override;
	};

}
