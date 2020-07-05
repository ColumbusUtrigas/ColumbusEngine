#pragma once

#include <Editor/Panel.h>
#include <Graphics/Texture.h>
#include <string>
#include <string_view>
#include <memory>

namespace Columbus
{

	class EditorPanelTexture : public EditorPanel
	{
	private:
		std::unique_ptr<Texture> _texture;
		std::string _path;
		bool _renable = true, _genable = true, _benable = true;
		int _mip = 0;
	public:
		EditorPanelTexture();

		void Load(const std::string_view path);
		void Draw();

		virtual ~EditorPanelTexture() final override;
	};

}
