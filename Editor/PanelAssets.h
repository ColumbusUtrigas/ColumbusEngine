#pragma once

#include <Editor/Panel.h>
#include <Core/FileInfo.h>
#include <list>
#include <string>

namespace Columbus
{

	class EditorPanelAssets : public EditorPanel
	{
	private:
		struct State
		{
			std::string path;

			State(const std::string& path) : path(path) {}
		};

		std::list<State> _history;
		int pointer = 0;
		
		State _current;
		FileInfo _popupElement;

		bool _goneTo = false;
		void _GoTo(const std::string& name);

		bool _CanGoBack();
		bool _CanGoForward();

		void _GoBack();
		void _GoForward();
	public:
		EditorPanelAssets();
		void Draw();
		virtual ~EditorPanelAssets() final override;
	};

}
