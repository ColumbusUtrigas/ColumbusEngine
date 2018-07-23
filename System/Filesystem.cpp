#include <System/Filesystem.h>
#include <System/System.h>

#if COLUMBUS_SYSTEM_WINDOWS
	#include <windows.h>
#endif

#ifdef COLUMBUS_SYSTEM_LINUX
	#include <dirent.h>
	#include <unistd.h>
	#include <fcntl.h>
	#include <sys/stat.h>
	#include <sys/types.h>
#endif

namespace Columbus
{

	/* 
	* Returns string to current work directory
	*/
	std::string Filesystem::getCurrent()
	{
		#ifdef COLUMBUS_SYSTEM_LINUX
			char dir[4096];
			if (getcwd(dir, 4096) == NULL) return "";
			return dir;
		#endif

		#ifdef COLUMBUS_SYSTEM_WINDOWS
			wchar_t wdir[MAX_PATH];
			char dir[MAX_PATH];
			GetCurrentDirectory(MAX_PATH, wdir);
			wcstombs(dir, wdir, MAX_PATH);
			return dir;
		#endif

		return "";
	}
	/* 
	* Creates new directory
	* @param aPath: Path to directory, where new directory should be created
	* @return Bool-value: true if directory successfully created, false if error was occured
	*/
	bool Filesystem::createDirectory(const std::string aPath)
	{
		#ifdef COLUMBUS_SYSTEM_LINUX
			return mkdir(aPath.c_str(), 0777) == 0;
		#endif

		#ifdef COLUMBUS_SYSTEM_WINDOWS
			wchar_t* name = new wchar_t[aPath.size()];
			mbstowcs(name, aPath.c_str(), aPath.size() * 2);
			return CreateDirectory(name, NULL);
		#endif

		return false;
	}
	/*
	* Creates new file
	* @param aPath: Path to directory, where new file should be created
	* @return Bool-value: true if file successfully created, false if error was occured
	*/
	bool Filesystem::createFile(const std::string aPath)
	{
		#ifdef COLUMBUS_SYSTEM_LINUX
			return open(aPath.c_str(), O_CREAT | O_EXCL | O_RDWR | O_CLOEXEC, S_IRWXU) >= 0;
		#endif

		//TODO: Windows

		return false;
	}
	/*
	* Removes directory
	* @param aPath: Path to directory, which should be removed
	* @return Bool-value: true if directory successfully removed, false if error was occured
	*/
	bool Filesystem::removeDirectory(const std::string aPath)
	{
		#ifdef COLUMBUS_SYSTEM_LINUX
			return rmdir(aPath.c_str()) == 0;
		#endif

		#ifdef COLUMBUS_SYSTEM_WINDOWS
			wchar_t* name = new wchar_t[aPath.size()];
			mbstowcs(name, aPath.c_str(), aPath.size() * 2);
			return RemoveDirectory(name);
		#endif

		return false;
	}
	/*
	* Removes file
	* @param aPath: Path to file, which should be removed
	* @return Bool-value: true if file successfully removed, false if error was occured
	*/
	bool Filesystem::removeFile(const std::string aPath)
	{
		return remove(aPath.c_str()) == 0;
	}
	/*
	* Changes directory or file name
	* @param aOld: Path to directory or file, which should be renamed
	* @param aNew: New name of directory or file
	* @return Bool-value: true if directory or file successfully renamed, false if error was occured
	*/
	bool Filesystem::rename(const std::string aOld, const std::string aNew)
	{
		return std::rename(aOld.c_str(), aNew.c_str()) == 0;
	}
	/*
	* Reads list of files and directories
	* @param aPath: Path to directory, which content should be returned
	* @return Vector of Strings: list of names of files and directories
	*/
	std::vector<std::string> Filesystem::read(const std::string aPath)
	{
		std::vector<std::string> ret;

		#ifdef COLUMBUS_SYSTEM_LINUX
			DIR* dir = opendir(aPath.c_str());
			if (dir == NULL) return ret;

			dirent* dptr = NULL;

			while ((dptr = readdir(dir)) != NULL)
			{
				ret.push_back(dptr->d_name);
			}

			closedir(dir);
		#endif

		#ifdef COLUMBUS_SYSTEM_WINDOWS
			HANDLE hFind;
			WIN32_FIND_DATA data;

			wchar_t* path = new wchar_t[aPath.size()];
			char file[MAX_PATH];
			mbstowcs(path, aPath.c_str(), aPath.size() * 2);

			std::wstring str = path;
			str += L"/*.*";

			hFind = FindFirstFile(str.c_str(), &data);

			if (hFind != INVALID_HANDLE_VALUE)
			{
				do
				{
					wcstombs(file, data.cFileName, MAX_PATH);
					ret.push_back(file);
				} while (FindNextFile(hFind, &data));
				FindClose(hFind);
			}
		#endif

		return ret;
	}
	/*
	* Reads list of files and directories in current
	* @return Vector of Strings: list of names of files and directories
	*/
	std::vector<std::string> Filesystem::readCurrent()
	{
		return read(getCurrent());
	}

}









