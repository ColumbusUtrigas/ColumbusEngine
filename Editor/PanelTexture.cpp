#include <Editor/PanelTexture.h>
#include <Editor/CommonUI.h>
#include <Graphics/Device.h>
#include <Core/Util.h>
#include <Lib/imgui/imgui.h>

namespace Columbus::Editor
{

	PanelTexture::PanelTexture()
	{

	}

	void PanelTexture::Load(const std::string_view path)
	{
		_path = path;
		if (!_texture)
		{
			_texture = std::unique_ptr<Texture>(gDevice->CreateTexture());
		}

		_texture->Load(_path.c_str());
	}

	void PanelTexture::Draw()
	{
		if (Opened)
		{
			if (ImGui::Begin("Texture", &Opened))
			{
				ImGui::PushID("PanelTexture");

				if (_texture)
				{
					uint64 size = ImageGetSize(_texture->GetWidth(), _texture->GetHeight(), 1, _texture->GetMipmapsCount(), _texture->GetFormat());
					double dsize;
					const char* ssize = HumanizeBytes(size, dsize);

					ImVec4 tint;

					ImGui::Text("%ix%i, %i mips, %s, %.1f %s", _texture->GetWidth(), _texture->GetHeight(), _texture->GetMipmapsCount(), TextureFormatToString(_texture->GetFormat()), dsize, ssize);
					ImGui::SameLine();

					ImGui::SameLine();
					FlagButton("R", _renable);
					ImGui::SameLine();
					FlagButton("G", _genable);
					ImGui::SameLine();
					FlagButton("B", _benable);
					ImGui::SameLine();

					ImGui::SetNextItemWidth(100);
					ImGui::SliderInt("Mip", &_mip, 0, _texture->GetMipmapsCount() - 1);
					_texture->SetMipmapLevel(_mip, 1000);

					if (_renable) tint.x = 1;
					if (_genable) tint.y = 1;
					if (_benable) tint.z = 1;
					tint.w = 1;

					ImGui::Image(_texture.get(), ImGui::GetWindowSize(), ImVec2(0, 0), ImVec2(1, 1), tint);
				}

				ImGui::PopID();
			}
			ImGui::End();
		}
	}

	PanelTexture::~PanelTexture()
	{

	}

}
