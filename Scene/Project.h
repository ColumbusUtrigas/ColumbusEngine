#pragma once

#include <string>
#include <vector>

namespace Columbus
{

	struct EngineProjectLevelEntry
	{
		std::string LevelName;

		// paths are relative to project base path
		std::string SourcePath;
		std::string ImportedPath;
	};

	struct EngineProject
	{
	public:
		static constexpr int DataVersion = 0;

		int Version;
		std::string ProjectName;
		std::string BasePath;

		int StartLevelIndex = -1;
		std::vector<EngineProjectLevelEntry> Levels;

	public:
		static EngineProject* CreateProject(std::string Name, std::string Path);
		static EngineProject* LoadProject(std::string Path);
		static void           SaveProject(const EngineProject* Project);
	};

	extern EngineProject* GCurrentProject;

}
