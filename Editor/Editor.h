#pragma once

#include <Scene/Scene.h>
#include <Editor/Settings.h>
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
#include <functional>

namespace Columbus::Editor
{

	class Editor
	{
	private:
		void DrawMainMenu(Scene& scene);
		void DrawToolbar();
		void DrawMainLayout(Scene& scene);

		float wheel = 0.0f;
		float cameraSpeed = 5.0f;
		bool wasLooking = false;
	public:
		Settings settings;

		PanelScene panelScene{ settings };
		PanelHierarchy panelHierarchy;
		PanelRenderSettings panelRenderSettings;
		PanelInspector panelInspector;
		PanelProfiler panelProfiler;
		PanelConsole panelConsole;
		PanelAssets panelAssets;
		std::shared_ptr<PanelTexture> panelTexture { new PanelTexture() };

		Camera camera;
	public:
		Editor();

		void Draw(Scene& scene, Renderer& Render, iVector2& Size, float RedrawTime);

		~Editor();
	};

}
