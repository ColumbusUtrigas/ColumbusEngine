#pragma once

#include "Types.h"
#include "Random.h"
#include "SmartPointer.h"
#include "Timer.h"
#include "Assert.h"
#include "String.h"

#include "Platform.h"
#include "Filesystem.h"
#include "Stacktrace.h"

#include "System/Log.h"
#include "System/File.h"

#define sizeofarray(a) (sizeof(a) / sizeof(a[0]))
#define COLUMBUS_EDITOR 1

// TODO: move it to somewhere else, it's not Core
void InitializeEngine();
std::string LoadShaderFile(const std::string& Name);

extern Columbus::u64 GFrameNumber;