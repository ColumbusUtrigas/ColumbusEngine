#include <Editor/ResourcesViewerShader.h>
#include <Editor/ResourcesViewerFunctions.h>
#include <Editor/FileDialog.h>
#include <Editor/MessageBox.h>
#include <Lib/imgui/imgui.h>
#include <Graphics/OpenGL/ShaderOpenGL.h>
#include <Graphics/Device.h>

namespace Columbus
{

	EditorFileDialog ShaderLoader("./Data/Shaders/");
	MessageBox ShaderBruteLoader("One or more shaders are already exist",
		"Do you realy want to load it against the already loaded ones?", { 300, 120 });

	bool ResourcesViewerShader::Opened = false;
	ShaderProgram** ResourcesViewerShader::Destination = nullptr;
	ShaderProgram* ResourcesViewerShader::Tmp = nullptr;

	void ResourcesViewerShader::Draw(Scene* Scn)
	{
		ShaderLoader.MultipleSelect(true);

		static std::string Find;

		if (Scn != nullptr && Opened)
		{
			ImGui::OpenPopup("Shaders Viewer");
			ImGui::SetNextWindowPosCenter(ImGuiCond_Always);
			ImGui::SetNextWindowSize(ImVec2(600, 370));
			if (ImGui::BeginPopupModal("Shaders Viewer", &Opened, ImGuiWindowFlags_NoResize))
			{
				if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_Escape))) Opened = false;

				auto LoadMore = [&]() { ShaderLoader.Open(); };
				auto Button = [&](const char* Name, void* _) { return ImGui::Button(Name, ImVec2(100, 100)); };
				auto RightClick = [&]() {};
				auto DoubleClick = [&]() { Close(); };

				auto Load = [&](const char* Path, ShaderProgram* S) { return S->Load(Path); };
				auto Success = [&](const char* _) { ShaderLoader.Close(); };
				auto Failure = [&]() { ShaderLoader.SetMessageBox(&ShaderBruteLoader); ShaderBruteLoader.Open(); };
				auto New = [&]() { return gDevice->CreateShaderProgram(); };

				ResourceViewerDrawLoadMore("LoadMore_ShadersViewer", LoadMore);
				ResourceViewerDrawList<ShaderProgram>("ShadersList_ShadersViewer", Tmp, Scn->ShadersManager, Find, Button, RightClick, DoubleClick);
				ResourceViewerDrawButtons("Buttons_ShadersViewer", Destination, Find, [&](){ Close(); }, Opened);
				ResourceViewerLoad<ShaderProgram>("Load Shader", ShaderLoader,
					Scn->ShadersManager, ShaderBruteLoader,
					Load, Success, Failure, New);

				ImGui::EndPopup();
			}
		}
	}

}


