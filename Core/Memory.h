#pragma once

#include <Core/Types.h>
#include <Core/Platform/Platform.h>
#include <cstdlib>
#include <cstring>

namespace Columbus
{

	struct Memory
	{
		static FORCEINLINE void* Malloc(uint32 Count) { return malloc(Count); }
		static FORCEINLINE void* Calloc(uint32 Num, uint32 Size) { return calloc(Num, Size); }
		static FORCEINLINE void* Realloc(void* Data, uint32 Count) { return realloc(Data, Count); }
		static FORCEINLINE void Free(void* Data) { free(Data); }
		static FORCEINLINE void Memset(void* Data, int32 Value, uint32 Count) { memset(Data, Value, Count); }
		static FORCEINLINE void Memcpy(void* Dst, const void* Src, uint32 Count) { memcpy(Dst, Src, Count); }
		static FORCEINLINE void Memmove(void* Dst, const void* Src, uint32 Count) { memmove(Dst, Src, Count); }
		static FORCEINLINE int Memcmp(const void* Data1, const void* Data2, uint32 Count) { return memcmp(Data1, Data2, Count); }
	};

}












