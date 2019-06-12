#pragma once

#include <string>

namespace Columbus
{

	struct FileInfo
	{
		std::string Name;
		std::string Ext;
		std::string Path;
		char Type;

		bool operator==(const FileInfo& Other) const
		{
			return Name == Other.Name && Ext == Other.Ext && Path == Other.Path && Type == Other.Type;
		}
	};

}


