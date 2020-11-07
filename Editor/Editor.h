#pragma once

#include <Scene/Scene.h>
#include <Editor/PanelScene.h>
#include <Editor/PanelHierarchy.h>
#include <Editor/PanelRenderSettings.h>
#include <Editor/Inspector/PanelInspector.h>
#include <Editor/PanelProfiler.h>
#include <Editor/PanelConsole.h>
#include <Editor/PanelAssets.h>
#include <Editor/PanelTexture.h>
#include <Graphics/Render.h>
#include <Math/Vector2.h>
#include <memory>

namespace Columbus::Editor
{

	class Editor
	{
	private:
		void DrawMainMenu(Scene& scene);
		void DrawDockSpace(Scene& scene);
	public:
		PanelScene panelScene;
		PanelHierarchy panelHierarchy;
		PanelRenderSettings panelRenderSettings;
		PanelInspector panelInspector;
		PanelProfiler panelProfiler;
		PanelConsole panelConsole;
		PanelAssets panelAssets;
		std::shared_ptr<PanelTexture> panelTexture { new PanelTexture() };
	public:
		Editor();

		void Draw(Scene& scene, Renderer& Render, iVector2& Size, float RedrawTime);

		~Editor();
	};

}
