/************************************************
*                   Console.h                   *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   21.07.2017                  *
*************************************************/

#pragma once

#include <cstdio>
#include <cstdlib>
#include <iostream>

#include <System/System.h>

namespace Columbus
{

  void C_Info(const char* fmt, ...);

  void C_Warning(const char* fmt, ...);

  void C_Error(const char* fmt, ...);

  void C_FatalError(const char* fmt, ...);

  void C_Success(const char* fmt, ...);

  void C_Initialization(const char* fmt, ...);

}
