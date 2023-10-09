#include "ShaderBytecode.h"
#include "Graphics/Core/Types.h"
#include <cstdint>
#include <cstdio>

struct CompiledShaderDataHeader
{
	uint32_t MagicNumber; // CCSD = 0x44534343 LE
	uint32_t Version;
	uint32_t Flags;
	uint32_t NumBytecodes;
};

struct CompiledShaderBytecodeHeader
{
	uint32_t Stage; // ShaderType 
	uint32_t Flags;
};

template <typename T>
static std::vector<T> ReadStdVector(FILE* f)
{
	uint32_t Size = 0;

	fread(&Size, sizeof(Size), 1, f);

	T* Data = (T*)malloc(Size * sizeof(T));
	fread(Data, sizeof(T), Size, f);

	std::vector<T> Result(Data, Data + Size);
	free(Data);
	return Result;
}

static std::string ReadString(FILE* f)
{
	static constexpr int StringBufSize = 2048;

	uint32_t Size = 0;
	char StringBuf[StringBufSize]{0};

	fread(&Size, sizeof(Size), 1, f);
	assert(Size < StringBufSize);
	fread(StringBuf, sizeof(char), Size, f);

	return std::string(StringBuf);
}

template <typename T>
static void WriteStdVector(const std::vector<T>& Vector, FILE* f)
{
	uint32_t Size = (uint32_t)Vector.size();

	fwrite(&Size, sizeof(Size), 1, f); // write size
	fwrite(Vector.data(), sizeof(T), Vector.size(), f); // write data
}

static void WriteString(const std::string& String, FILE* f)
{
	uint32_t Size = (uint32_t)String.size();

	fwrite(&Size, sizeof(Size), 1, f); // write size
	fwrite(String.c_str(), sizeof(char), String.size(), f); // write string, not null-terminated
}

CompiledShaderData LoadCompiledShaderData(const std::string& Path)
{
	CompiledShaderData Result;

	CompiledShaderDataHeader DataHeader;

	FILE* f = fopen(Path.c_str(), "rb");

	fread(&DataHeader, sizeof(DataHeader), 1, f);
	Result.Name = ReadString(f);

	// TODO: error handling

	for (int i = 0; i < DataHeader.NumBytecodes; i++)
	{
		CompiledShaderBytecode Bytecode;
		CompiledShaderBytecodeHeader BytecodeHeader;

		fread(&BytecodeHeader, sizeof(BytecodeHeader), 1, f);
		Bytecode.Stage = (Columbus::ShaderType)BytecodeHeader.Stage;
		Bytecode.Flags = BytecodeHeader.Flags;

		Bytecode.EntryPoint = ReadString(f);
		Bytecode.Bytecode = ReadStdVector<uint8_t>(f);
	}

	fclose(f);

	return Result;
}

void SaveCompiledShaderData(const CompiledShaderData& Data, const std::string& Path)
{
	CompiledShaderDataHeader DataHeader {
		.MagicNumber  = 0x44534343,
		.Version      = 1,
		.Flags        = Data.Flags,
		.NumBytecodes = (uint32_t)Data.Bytecodes.size(),
	};

	FILE* f = fopen(Path.c_str(), "wb");

	fwrite(&DataHeader, sizeof(DataHeader), 1, f);
	WriteString(Data.Name, f);

	for (const auto& Bytecode : Data.Bytecodes)
	{
		CompiledShaderBytecodeHeader BytecodeHeader{
			.Stage = (uint32_t)Bytecode.Stage,
			.Flags = Bytecode.Flags
		};

		fwrite(&BytecodeHeader, sizeof(BytecodeHeader), 1, f);
		WriteString(Bytecode.EntryPoint, f);
		WriteStdVector<uint8_t>(Bytecode.Bytecode, f);
	}

	fclose(f);
}