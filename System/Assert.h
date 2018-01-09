#pragma once

#include <time.h>
#include <math.h>
#include <float.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <typeinfo>

#include <System/Log.h>

namespace Columbus
{

	#define COLUMBUS_ASSERT(EXP) { EXP ? (void)0 : C_Log::fatal("%s:%d Assertation: %s", __FILE__, __LINE__, #EXP); }
	#define COLUMBUS_ASSERT_MESSAGE(EXP, MES) { EXP ? (void)0 : C_Log::fatal("%s:%d Assertation: %s", __FILE__, __LINE__, #MES); }

}




