#pragma once

#include <string>
#include <vector>

namespace Columbus
{

	class Filesystem
	{
	public:
		static std::string getCurrent();
		static bool createDirectory(const std::string aPath);
		static bool createFile(const std::string aPath);
		static bool removeDirectory(const std::string aPath);
		static bool removeFile(const std::string aPath);
		static bool renameDirectory(const std::string aOld, const std::string aNew);
		static bool renameFile(const std::string aOld, const std::string aNew);
		static std::vector<std::string> read(const std::string aPath);
		static std::vector<std::string> readCurrent();
	};

}















