#include "Main.h"
#include <imgui/imgui.h>

/*bool InitializeEngine()
{
	InitializeCommon();

	switch (chosen_api)
	{
	case Columbus::GraphicsAPI::None:
		MessageBox(0, "Chosen Graphics API: None", "Error", MB_APPLMODAL | MB_ICONERROR | MB_OK);
		return false;

	case Columbus::GraphicsAPI::OpenGL:
		if (!InitializeWindowAndContext_GL())
		{
			MessageBox(0, "GL Initialization Failed", "Error", MB_APPLMODAL | MB_ICONERROR | MB_OK);
			return false;
		}

		InitializeImGUI_GL();
		return true;

	case Columbus::GraphicsAPI::DX12:
		if (!InititalizeWindowAndContext_DX12())
		{
			MessageBox(0, "DX12 Initialization Failed", "Error", MB_APPLMODAL | MB_ICONERROR | MB_OK);
			return false;
		}

		InitializeImGUI_DX12();
		return true;

	case Columbus::GraphicsAPI::Vulkan:
		if (!InitializeWindowAndContext_VK())
		{
			MessageBox(0, "Vulkan Initialization Failed", "Error", MB_APPLMODAL | MB_ICONERROR | MB_OK);
			return false;
		}

		return true;
	}

	return false;
}

void NewFrameImGUI()
{
	switch (chosen_api)
	{
	case Columbus::GraphicsAPI::None:
		break;

	case Columbus::GraphicsAPI::OpenGL:
		NewFrameImGUI_GL();
		break;

	case Columbus::GraphicsAPI::DX12:
		NewFrameImGUI_DX12();
		break;

	case Columbus::GraphicsAPI::Vulkan:
		break;
	}
}

void BeginFrame()
{
	switch (chosen_api)
	{
	case Columbus::GraphicsAPI::None:
		break;

	case Columbus::GraphicsAPI::OpenGL:
		BeginFrame_GL();
		break;

	case Columbus::GraphicsAPI::DX12:
		BeginFrame_DX12();
		break;

	case Columbus::GraphicsAPI::Vulkan:
		BeginFrame_VK();
		break;
	}
}

void Render()
{
	switch (chosen_api)
	{
	case Columbus::GraphicsAPI::None:
		break;

	case Columbus::GraphicsAPI::OpenGL:
		RenderImGUI_GL();
		Present_GL();
		break;

	case Columbus::GraphicsAPI::DX12:
		//Render_DX12();
		RenderImGUI_DX12();
		Present_DX12();
		break;

	case Columbus::GraphicsAPI::Vulkan:
		Present_VK();
		break;
	}
}

void ShutdownEngine()
{
	switch (chosen_api)
	{
	case Columbus::GraphicsAPI::None:
		break;

	case Columbus::GraphicsAPI::OpenGL:
		ShutdownImGUI_GL();
		ShutdownWindowAndContext_GL();
		break;

	case Columbus::GraphicsAPI::DX12:
		ShutdownImGUI_DX12();
		ShutdownWindowAndContext_DX12();
		break;

	case Columbus::GraphicsAPI::Vulkan:
		ShutdownWindowAndContext_VK();
		break;
	}
}

void InitializeCommon()
{
	hinstance = GetModuleHandle(NULL);
	InitializeImGUI_Common();
}

void InitializeImGUI_Common()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	//io.WantSaveIniSettings = false;
	io.IniFilename = "interface.ini";
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
}*/
