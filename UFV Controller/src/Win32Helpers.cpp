#include "Win32Helpers.h"

void Win32Log(const char* format, ...)
{
	char buf[0x1000] = {};

	char* argp = (char*)&format + sizeof(format);

	vsprintf_s(buf, 0x1000, format, argp);

	strcat_s(buf, 0x1000, "\r\n");

	argp = nullptr;

	OutputDebugStringA(buf);
}

std::string Win32GetErrorCodeDescription(DWORD err)
{
	std::string result;

	char* buffer = nullptr;
	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (char*)&buffer, 0, NULL);

	if (buffer)
	{
		// replace first CRLF with null terminator
		for (int i = 0; buffer && buffer[i]; ++i)
		{
			if (buffer[i] == '\r')
			{
				if (buffer[i + 1] == '\n')
				{
					buffer[i] = 0;
					break;
				}
			}
		}
		result = buffer;
		LocalFree(buffer);
	}
	else
	{
		result = "failed to get error code description";
	}

	return result;
}

HANDLE Win32OpenAndConfigureComPort(const char* name)
{
	printf("Win32OpenAndConfigureComPort() connecting to %s\n", name);
	HANDLE serial = CreateFileA(name, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (!serial || serial == INVALID_HANDLE_VALUE)
	{
		Win32Log("[Win32OpenAndConfigureComPort] CreateFile() failed with error %d: %s\n", GetLastError(), Win32GetErrorCodeDescription(GetLastError()).c_str());
		return 0;
	}

	DCB dcb = {};

	if (!GetCommState(serial, &dcb))
	{
		Win32Log("[Win32OpenAndConfigureComPort] GetCommState() failed with error %d: %s\n", GetLastError(), Win32GetErrorCodeDescription(GetLastError()).c_str());
		return 0;
	}

	dcb.BaudRate = CBR_9600;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;
	dcb.ByteSize = 8;

	if (!SetCommState(serial, &dcb))
	{
		Win32Log("[Win32OpenAndConfigureComPort] SetCommState() failed with error %d: %s\n", GetLastError(), Win32GetErrorCodeDescription(GetLastError()).c_str());
		return 0;
	}

	return serial;
}
