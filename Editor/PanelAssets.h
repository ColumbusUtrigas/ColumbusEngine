#pragma once

#include <Editor/Panel.h>
#include <Editor/PanelTexture.h>
#include <Core/FileInfo.h>
#include <list>
#include <string>

namespace Columbus::Editor
{

	class PanelAssets : public EditorPanel
	{
	public:
		PanelAssets();

		void SetTexturePreview(std::weak_ptr<PanelTexture> preview);
		void Draw();

		virtual ~PanelAssets() final override;

	private:
		std::weak_ptr<PanelTexture> _texturePreview;

	private:
		const char* const _popupName = "ItemPopup";
		FileInfo _popupElement;

		struct State
		{
			std::string path;

			State(const std::string& path) : path(path) {}
		};

		std::list<State> _history;
		State _current;
		int pointer = 0;
		bool _goneTo = false;

	private:
		void _GoTo(const std::string& name);

		bool _CanGoBack();
		bool _CanGoForward();

		void _GoBack();
		void _GoForward();

		void _OpenPopup(const FileInfo& info);
		void _DrawPopup();
	};

}
