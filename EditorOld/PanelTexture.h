#pragma once

#include <Editor/Panel.h>
#include <Graphics/Texture.h>
#include <string>
#include <string_view>
#include <memory>

namespace Columbus::Editor
{

	class PanelTexture : public Panel
	{
	private:
		std::unique_ptr<Texture> _texture;
		std::string _path;
		bool _renable = true, _genable = true, _benable = true;
		int _mip = 0;

		void DrawInternal() final override;
	public:
		PanelTexture();

		void Load(const std::string_view path);

		virtual ~PanelTexture() final override;
	};

}
