#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <cstdio>
#include <string>

void Win32Log(const char* format, ...);

std::string Win32GetErrorCodeDescription(DWORD err);

HANDLE Win32OpenAndConfigureComPort(const char* name);