#pragma once

namespace Columbus
{
	enum class ShowInExplorerType
	{
		Root,
		Select
	};

	void EngineOpenFileDialog();
	void EngineShowInFileExplorer(const char* path, ShowInExplorerType type);
}
