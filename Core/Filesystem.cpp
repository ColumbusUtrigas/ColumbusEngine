#include <Core/Filesystem.h>
#include <stdio.h>

namespace Columbus
{

	String Filesystem::RelativePath(const String& Absolute, const String& RelativeTo)
	{
		String Abs = Absolute;
		String Rel = RelativeTo;
		for (auto& Ch : Abs) if (Ch == '\\') Ch = '/';
		for (auto& Ch : Rel) if (Ch == '\\') Ch = '/';

		auto AbsDecompose = Split(Abs);
		auto RelDecompose = Split(Rel);

		String Result;

		if (AbsDecompose.size() >= RelDecompose.size())
		{
			std::vector<String> ResDecompose = AbsDecompose;
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

	std::vector<String> Filesystem::Split(const String& Path)
	{
		String MPath = Path;
		std::vector<String> Result;

		for (auto& Ch : MPath)
			if (Ch == '\\')
				Ch = '/';

		size_t Start = 0;
		size_t Pos = 0;
		String Token;
		while ((Pos = MPath.find('/', Start)) != String::npos)
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


