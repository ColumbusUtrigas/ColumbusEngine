#pragma once

void SetupSystemCrashHandler();
void WriteStacktraceToLog();

// test crash functions
void Crash_NullPointer();
void Crash_InvalidWrite();
void Crash_StackOverflow();
void Crash_DivideByZero();
void Crash_DebugBreak();
