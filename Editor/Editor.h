#pragma once

#include <Scene/Scene.h>
#include <Editor/PanelScene.h>
#include <Editor/PanelHierarchy.h>
#include <Editor/PanelRenderSettings.h>
#include <Editor/Inspector/PanelInspector.h>
#include <Editor/PanelProfiler.h>
#include <Editor/PanelConsole.h>
#include <Graphics/Render.h>

#include <Math/Vector2.h>

namespace Columbus
{

	class Editor
	{
	private:
		void ApplyDarkTheme();
		void DrawMainMenu(Scene& scene);
		void DrawDockSpace(Scene& scene);
	public:
		EditorPanelScene PanelScene;
		EditorPanelHierarchy PanelHierarchy;
		EditorPanelRenderSettings PanelRenderSettings;
		EditorPanelInspector PanelInspector;
		EditorPanelProfiler PanelProfiler;
		EditorPanelConsole PanelConsole;
	public:
		Editor();

		void Draw(Scene& scene, Renderer& Render, iVector2& Size, float RedrawTime);

		~Editor();
	};

}


