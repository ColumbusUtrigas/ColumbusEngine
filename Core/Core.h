#pragma once

#include "Types.h"
#include "Random.h"
#include "Reflection.h"
#include "SmartPointer.h"
#include "Timer.h"
#include "Assert.h"
#include "String.h"

#include "CommandLine.h"
#include "Platform.h"
#include "Filesystem.h"
#include "Stacktrace.h"

#include "System/Log.h"
#include "System/File.h"

#define sizeofarray(a) (sizeof(a) / sizeof(a[0]))
#define COLUMBUS_EDITOR 1

void InitializeEngine(int argc, char** argv);

// TODO: move it to somewhere else, it's not Core
std::string LoadShaderFile(const std::string& Name);

extern Columbus::u64 GFrameNumber;