#include <Editor/PanelAssets.h>
#include <Editor/FontAwesome.h>
#include <Editor/Icons.h>
#include <Editor/CommonUI.h>
#include <Lib/imgui/imgui.h>
#include <Lib/imgui/imgui_internal.h>
#include <Common/JSON/JSON.h>
#include <Common/base64/base64.h>
#include <Core/Filesystem.h>
#include <Core/FileDialog.h>

#include <fstream>
#include <unordered_map>
#include <Graphics/OpenGL/ShaderOpenGL.h>
#include <Graphics/Device.h>
#include <Graphics/ScreenQuad.h>

#include <Lib/stb/stb_dxt.h>
#include <Lib/stb/stb_image_resize.h>

namespace Columbus
{

	struct IconCache
	{
		std::unordered_map<std::string, Texture*> _cache;
		std::string _cachePath = "./EngineCache/Assets/TexIcons";
		bool _created = false;

		bool Has(const std::string& path)
		{
			bool alreadyHas = _cache.find(path) != _cache.end();
			if (alreadyHas) return true;

			if (!_created)
				Filesystem::CreateDirectories(_cachePath);
			_created = true;

			auto cachePath = _cachePath + '/' + path + ".json";
			if (Filesystem::Exists(cachePath))
			{
				JSON J;
				if (J.Load(cachePath.c_str()))
				{
					auto w = J["Width"].GetInt();
					auto h = J["Height"].GetInt();
					auto b64 = J["Data"].GetString();

					size_t outlen;
					unsigned char* data = base64_decode(b64.data(), b64.length(), &outlen);

					TextureDesc desc(w, h, 0, 0, TextureFormat::RGBA8);
					_cache[path] = gDevice->CreateTexture();
					_cache[path]->Create2D(desc);
					_cache[path]->Load(data, desc);
					_cache[path]->SetFlags(Texture::Flags(Texture::Filter::Linear, Texture::Anisotropy::Anisotropy1, Texture::Wrap::Clamp));
					_cache[path]->SetMipmapLevel(0, 0);
					return true;
				}
			}

			return false;
		}

		void Load(const std::string& path)
		{
			if (!Has(path))
			{
				auto tex = gDevice->CreateTexture();
				tex->Load(path.c_str());

				TextureDesc desc(16, 16, 0, 0, TextureFormat::RGBA8);

				_cache[path] = gDevice->CreateTexture();
				_cache[path]->Create2D(desc);

				ScreenQuad quad;
				auto shader = static_cast<ShaderProgramOpenGL*>(gDevice->GetDefaultShaders()->ScreenSpace.get());
				shader->Bind();
				shader->SetUniform("BaseTexture", tex, 0);

				auto buf = gDevice->CreateFramebuffer();
				buf->Bind();
				glViewport(0, 0, 16, 16);
				buf->SetTexture2D(Framebuffer::Attachment::Color0, _cache[path]);
				quad.Render();
				buf->Unbind();

				auto cachetex = static_cast<TextureOpenGL*>(_cache[path]);
				int pixelsSize = 16 * 16 * 4;
				unsigned char pixels[16 * 16 * 4];
				cachetex->SetFlags(Texture::Flags(Texture::Filter::Linear, Texture::Anisotropy::Anisotropy1, Texture::Wrap::Clamp));
				cachetex->SetMipmapLevel(0, 0);

				cachetex->Bind();
				glGetTexImage(cachetex->GetTarget(), 0, cachetex->GetPixelFormat(), cachetex->GetPixelType(), pixels);
				cachetex->Unbind();

				auto imgPath = _cachePath + '/' + path;
				auto split = Filesystem::Split(imgPath);
				split.erase(split.end() - 1);
				imgPath.clear();

				for (auto& s : split)
					imgPath += s + '/';

				Filesystem::CreateDirectories(imgPath);

				imgPath = _cachePath + '/' + path + ".json";

				size_t outlen;
				char* data = base64_encode(pixels, pixelsSize, &outlen);

				JSON J;
				J["Width"] = 16;
				J["Height"] = 16;
				J["Data"] = std::string(data, outlen);
				J.Save(imgPath.c_str());

				delete buf;
				delete tex;
			}
		}

		Texture* Get(const std::string& path)
		{
			if (Has(path))
				return _cache[path];

			return nullptr;
		}
	};


	EditorPanelAssets::EditorPanelAssets() :
		_current("Data/")
	{
		_history.push_back(_current);
	}

	IconCache gIconCache;
	std::string textpath;
	bool textprev = false;
	bool loaded = false;

	void EditorPanelAssets::SetTexturePreview(std::weak_ptr<EditorPanelTexture> preview)
	{
		_texturePreview = preview;
	}

	void EditorPanelAssets::Draw()
	{
		if (Opened)
		{
			_goneTo = false;
			loaded = false;

			ImGui::PushID("EditorPanelAssets");
			if (ImGui::Begin(ICON_FA_FOLDER" Assets", &Opened, ImGuiWindowFlags_NoCollapse))
			{
				auto files = Filesystem::Read(_current.path);

				if (ImGui::BeginChild("AssetsToolbar", ImVec2(ImGui::GetWindowContentRegionWidth(), 25)))
				{
					ImGui::PushItemFlag(ImGuiItemFlags_Disabled, !_CanGoBack());
					if (ImGui::Button(ICON_FA_ARROW_LEFT))
						_GoBack();
					ImGui::PopItemFlag();

					ImGui::PushItemFlag(ImGuiItemFlags_Disabled, !_CanGoForward());
					ImGui::SameLine();
					if (ImGui::Button(ICON_FA_ARROW_RIGHT))
						_GoForward();
					ImGui::PopItemFlag();

					static char buf[1024] = { 0 };
					ImGui::SameLine();
					ImGui::InputText("##Find", buf, 1024);
				}
				ImGui::EndChild();

				if (ImGui::BeginChild("AssetsList"))
				{
					for (auto& file : files)
					{
						if (file.Name == "." || file.Name == "..") continue;

						std::string icon;

						switch (file.Type)
						{
						case 'd': icon = ICON_FA_FOLDER; break;
						case 'f': icon = GetFileIcon(file.Ext); break;
						}
						icon += ' ';

						if (Filesystem::IsImage(file.Ext))
						{
							if (!gIconCache.Has(file.Path))
							{
								if (!loaded)
								{
									gIconCache.Load(file.Path);
									loaded = true;
								}
							}
							else
							{
								icon = "";
								ImGui::Image(gIconCache.Get(file.Path), ImVec2(ImGui::GetFontSize(), ImGui::GetFontSize()));
								ImGui::SameLine();
							}
						}

						ImGui::Selectable((icon + file.Name).c_str());

						if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
						{
							if (file.Type == 'd')
								_GoTo(file.Name);

							if (file.Type == 'f')
							{
								if (Filesystem::IsImage(file.Ext))
								{
									if (!_texturePreview.expired())
									{
										auto _preview = _texturePreview.lock();
										_preview->Load(file.Path);
									}
								}
								else
								{
									std::ifstream f(file.Path.c_str());
									textpath = std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
									textprev = true;
								}
							}
						}

						if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1))
						{
							_OpenPopup(file);
						}
					}
				}

				_DrawPopup();
				ImGui::EndChild();
			}

			ImGui::End();
			ImGui::PopID();
		}

		if (textprev)
		{
			if (ImGui::Begin("File Preview", &textprev))
			{
				ImGui::TextUnformatted(textpath.c_str());
			}
			ImGui::End();
		}
	}

	void EditorPanelAssets::_GoTo(const std::string& name)
	{
		if (!_goneTo)
		{
			_current.path += name + '/';
			_history.push_back(_current);
			pointer++;
			_goneTo = true;
		}
	}

	bool EditorPanelAssets::_CanGoBack()
	{
		return pointer > 0;
	}

	bool EditorPanelAssets::_CanGoForward()
	{
		return pointer < (_history.size() - 1);
	}

	void EditorPanelAssets::_GoBack()
	{
		if (_CanGoBack())
		{
			pointer--;
			auto it = _history.begin();
			std::advance(it, pointer);
			_current = *it;
		}
	}

	void EditorPanelAssets::_GoForward()
	{
		if (_CanGoForward())
		{
			pointer++;
			auto it = _history.begin();
			std::advance(it, pointer);
			_current = *it;
		}
	}

	void EditorPanelAssets::_OpenPopup(const FileInfo& info)
	{
		ImGui::OpenPopup(_popupName);
		_popupElement = info;
	}

	void EditorPanelAssets::_DrawPopup()
	{
		if (ImGui::BeginPopup(_popupName))
		{
			ImGui::PushID(_popupName);
			if (ImGui::Selectable("Show in File Explorer"))
			{
				EngineShowInFileExplorer(_popupElement.Path.c_str(), ShowInExplorerType::Select);
			}

			ImGui::EndPopup();
			ImGui::PopID();
		}
	}

	EditorPanelAssets::~EditorPanelAssets()
	{

	}

}