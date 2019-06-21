#include <Core/Platform/Platform.h>
#include <Core/Platform/PlatformFilesystem.h>
#include <cstdio>
#include <cstring>

#if defined(PLATFORM_WINDOWS)
	#include <Core/Windows/PlatformWindowsFilesystem.h>
#elif defined(PLATFORM_LINUX)
	#include <Core/Linux/PlatformLinuxFilesystem.h>
#endif

namespace Columbus
{

	std::string Filesystem::GetCurrent()
	{
		#if defined(PLATFORM_WINDOWS)
			return FilesystemWindows::GetCurrent();
		#elif defined(PLATFORM_LINUX)
			return FilesystemLinux::GetCurrent();
		#endif

		return "";
	}

	std::string Filesystem::AbsolutePath(const std::string& Path)
	{
		#if defined(PLATFORM_WINDOWS)
			return FilesystemWindows::AbsolutePath(Path);
		#elif defined(PLATFORM_LINUX)
			return FilesystemLinux::AbsolutePath(Path);
		#endif

		return "";
	}

	std::string Filesystem::RelativePath(const std::string& Absolute, const std::string& RelativeTo)
	{
		std::string Abs = Absolute;
		std::string Rel = RelativeTo;
		for (auto& Ch : Abs) if (Ch == '\\') Ch = '/';
		for (auto& Ch : Rel) if (Ch == '\\') Ch = '/';

		auto AbsDecompose = Split(Abs);
		auto RelDecompose = Split(Rel);

		std::string Result;

		if (AbsDecompose.size() >= RelDecompose.size())
		{
			std::vector<std::string> ResDecompose = AbsDecompose;
			for (auto _ : RelDecompose) ResDecompose.erase(ResDecompose.begin());

			for (const auto& Elem : ResDecompose)
			{
				Result += Elem + "/";
			}

			Result.pop_back();
		} else
		{
			for (size_t i = 0; i <= (RelDecompose.size() - AbsDecompose.size()); i++)
			{
				Result += "../";
			}

			Result += AbsDecompose[AbsDecompose.size() - 1];
		}

		return Result;
	}

	bool Filesystem::FileCreate(const char* Path)
	{
		FILE* File = fopen(Path, "w");
		bool Result = File != nullptr;
		if (File != nullptr) fclose(File);
		return Result;
	}

	bool Filesystem::DirCreate(const char* Path)
	{
		#if defined(PLATFORM_WINDOWS)
			return FilesystemWindows::DirCreate(Path);
		#elif defined(PLATFORM_LINUX)
			return FilesystemLinux::DirCreate(Path);
		#endif

		return false;
	}

	bool Filesystem::Rename(const char* Old, const char* New)
	{
		return rename(Old, New) == 0;
	}

	bool Filesystem::FileRemove(const char* Path)
	{
		return remove(Path) == 0;
	}

	bool Filesystem::DirRemove(const char* Path)
	{
		#if defined(PLATFORM_WINDOWS)
			return FilesystemWindows::DirRemove(Path);
		#elif defined(PLATFORM_LINUX)
			return FilesystemLinux::DirRemove(Path);
		#endif

		return false;
	}

	std::vector<FileInfo> Filesystem::Read(const std::string& Path)
	{
		#if defined(PLATFORM_WINDOWS)
			return FilesystemWindows::Read(Path);
		#elif defined(PLATFORM_LINUX)
			return FilesystemLinux::Read(Path);
		#endif
		
		return {};
	}

	std::vector<std::string> Filesystem::Split(const std::string& Path)
	{
		std::string MPath = Path;
		std::vector<std::string> Result;

		for (auto& Ch : MPath)
			if (Ch == '\\')
				Ch = '/';

		size_t Start = 0;
		size_t Pos = 0;
		std::string Token;
		while ((Pos = MPath.find("/", Start)) != std::string::npos)
		{
			Token = MPath.substr(Start, Pos - Start);
			if (Token.empty())
				Result.push_back("/");
			else
				Result.push_back(Token);
			Start = Pos + 1;
		}

		if (!Path.substr(Start).empty())
			Result.push_back(Path.substr(Start));

		return Result;
	}

}


