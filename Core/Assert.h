#pragma once

#include <Core/Platform.h>
#include <System/Log.h>
#include <cassert>

void _ColumbusAssert_Internal(const char* file, long int line, const char* msg);

#define COLUMBUS_ASSERT(EXP) if (!(EXP)) { DEBUGBREAK(); _ColumbusAssert_Internal(__FILE__, __LINE__, #EXP); assert(false); }
#define COLUMBUS_ASSERT_MESSAGE(EXP, MES) if (!(EXP)) { DEBUGBREAK(); _ColumbusAssert_Internal(__FILE__, __LINE__, MES); assert(false); }
