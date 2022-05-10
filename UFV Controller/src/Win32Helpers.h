#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <cstdio>
#include <string>

namespace LogCallback
{
#define LOG_CALLBACK_FN(name) void name(const char* str, void* parameter);
	typedef LOG_CALLBACK_FN(log_callback_fn);

	extern log_callback_fn* callback;
	extern void* callbackParameter;
}

void Win32Log(const char* format, ...);

std::string Win32GetErrorCodeDescription(DWORD err);

HANDLE Win32OpenAndConfigureComPort(const char* name);

bool Win32WriteByteToComPort(HANDLE port, char byte);