#include <Core/Filesystem.h>
#include <dirent.h>
#include <unistd.h>

namespace Columbus
{

	String Filesystem::GetCurrent()
	{
		char dir[4096];
		if (getcwd(dir, 4096) == NULL) return "";
		return dir;
	}

	String Filesystem::AbsolutePath(const String& Path)
	{
		char dir[PATH_MAX];
		return realpath(Path.c_str(), dir);
	}

	std::vector<FileInfo> Filesystem::Read(const String& Path)
	{
		std::vector<FileInfo> Result;

		DIR* dir = opendir(Path.c_str());
		if (dir == NULL) return Result;

		dirent* dptr = NULL;

		while ((dptr = readdir(dir)) != NULL)
		{
			FileInfo Info;

			String Name = dptr->d_name;
			unsigned char Type = dptr->d_type;

			auto Pos = Name.rfind('.');

			Info.Name = Name;
			Info.Ext = (Pos != String::npos && Pos != 0) ? Name.substr(Pos + 1) : "";
			Info.Path = Path[Path.length() - 1] == '/' ? Path + Name : Path + '/' + Name;

			switch (Type)
			{
			case DT_DIR: Info.Type = 'd'; break;
			case DT_LNK: Info.Type = 'l'; break;
			case DT_REG: Info.Type = 'f'; break;
			}
			
			Result.push_back(Info);
		}

		closedir(dir);

		return Result;
	}

}


