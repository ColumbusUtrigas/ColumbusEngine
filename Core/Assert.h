#pragma once

#include <System/Log.h>

void _ColumbusAssert_Internal(const char* file, long int line, const char* msg);

#define COLUMBUS_ASSERT(EXP) if (!(EXP)) _ColumbusAssert_Internal(__FILE__, __LINE__, #EXP);
#define COLUMBUS_ASSERT_MESSAGE(EXP, MES) if (!(EXP)) _ColumbusAssert_Internal(__FILE__, __LINE__, MES);
