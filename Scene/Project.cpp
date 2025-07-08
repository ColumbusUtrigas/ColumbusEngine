#include "Project.h"
#include "Core/Core.h"

#include <iomanip>
#include <fstream>
#include <filesystem>
#include "Lib/json/single_include/nlohmann/json.hpp"

namespace Columbus
{

	EngineProject* GCurrentProject = nullptr;
	std::string    GCurrentLevelPath = "";

	EngineProject* EngineProject::CreateProject(std::string Name, std::string Path)
	{
		EngineProject* Project = new EngineProject
		{
			.ProjectName = Name,
			.BasePath = Path,
		};

		SaveProject(Project);

		return Project;
	}

	EngineProject* EngineProject::LoadProject(std::string Path)
	{
		EngineProject* Project = new EngineProject();

		nlohmann::json json;
		std::ifstream ifs(Path);
		ifs >> json;

		auto Base = std::filesystem::path(Path).parent_path();

		Project->BasePath = Base.string();
		Project->DataPath = (Base / "Data").string();

		Project->Version = json["Version"];
		Project->ProjectName = json["ProjectName"];
		Project->StartLevelIndex = json["StartLevelIndex"];

		for (auto& Element : json["Levels"].array())
		{
			EngineProjectLevelEntry Level;
			Level.LevelName = Element["LevelName"];
			Level.ImportedPath = Element["ImportedPath"];
			Level.SourcePath = Element["SourcePath"];
		}

		if (Project->Version != EngineProject::DataVersion)
		{
			Log::Fatal("Project version doesn't match. Loaded %i, required %i", Project->Version, EngineProject::DataVersion);
		}

		return Project;
	}

	void EngineProject::SaveProject(const EngineProject* Project)
	{
		nlohmann::json json;
		json["Version"] = Project->Version;
		json["ProjectName"] = Project->ProjectName;
		json["StartLevelIndex"] = Project->StartLevelIndex;

		for (int i = 0; i < (int)Project->Levels.size(); i++)
		{
			json["Levels"][i]["LevelName"] = Project->Levels[i].LevelName;
			json["Levels"][i]["ImportedPath"] = Project->Levels[i].ImportedPath;
			json["Levels"][i]["SourcePath"] = Project->Levels[i].SourcePath;
		}

		std::ofstream ofs(Project->BasePath + "/Project.json");
		ofs << std::setw(4) << json;
	}

}
