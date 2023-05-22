#pragma once

#include "Types.h"
#include "Random.h"
#include "SmartPointer.h"
#include "Timer.h"
#include "Assert.h"

#include "Platform.h"
#include "Filesystem.h"
#include "Stacktrace.h"

#define sizeofarray(a) (sizeof(a) / sizeof(a[0]))
#define COLUMBUS_EDITOR 1

void InitializeEngine();
