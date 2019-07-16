#pragma once

#include <Core/String.h>

namespace Columbus
{

	struct FileInfo
	{
		String Name;
		String Ext;
		String Path;
		char Type;

		bool operator==(const FileInfo& Other) const
		{
			return Name == Other.Name && Ext == Other.Ext && Path == Other.Path && Type == Other.Type;
		}
	};

}


