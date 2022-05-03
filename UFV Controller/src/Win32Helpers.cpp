#include "Win32Helpers.h"

void Win32Log(const char* format, ...)
{
	char buf[0x1000] = {};

	char* argp = (char*)&format + sizeof(format);
	vsprintf_s(buf, 0x1000, format, argp);
	argp = nullptr;

	strcat_s(buf, 0x1000, "\r\n");

	printf("%s", buf);
	OutputDebugStringA(buf);

	char path[MAX_PATH + 1] = {};
	if (GetCurrentDirectoryA(MAX_PATH + 1, path))
	{
		if (!strcat_s(path, MAX_PATH + 1, "\\log.txt"))
		{
			HANDLE file = 0;

			if (GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES)
				file = CreateFileA(path, FILE_APPEND_DATA, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
			else if (GetLastError() == 2) // the system cannot find the file specified
				file = CreateFileA(path, GENERIC_WRITE, 0, 0, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 0);

			if (file && file != INVALID_HANDLE_VALUE)
			{
				DWORD written = 0;
				WriteFile(file, buf, strlen(buf), &written, 0);
				CloseHandle(file);
			}
		}
	}
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

// could consider using TransmitCommChar instead of WriteFile
bool Win32WriteByteToComPort(HANDLE port, char byte)
{

	DWORD writeCount = 0;
	bool success = WriteFile(port, &byte, 1, &writeCount, 0) & 1;

	if (!success)
		Win32Log("[Win32WriteByteToComPort] WriteFile() failed with error %d: %s", GetLastError(), Win32GetErrorCodeDescription(GetLastError()).c_str());

	// WriteFile will return true even if it times out.
	// If it times out, then it will not have written
	// the single byte, so writeCount will be zero
	success = success && writeCount == 1;

	DWORD errors = 0;
	if (!ClearCommError(port, &errors, 0))
		Win32Log("[Win32WriteByteToComPort] ClearCommError() failed with error %d: %s", GetLastError(), Win32GetErrorCodeDescription(GetLastError()).c_str());

	if (errors)
	{
		if (errors & CE_BREAK)
		{
			Win32Log("[Win32WriteByteToComPort] Comm Error: CE_BREAK");
		}
		if (errors & CE_FRAME)
		{
			Win32Log("[Win32WriteByteToComPort] Comm Error: CE_FRAME");
		}
		if (errors & CE_OVERRUN)
		{
			Win32Log("[Win32WriteByteToComPort] Comm Error: CE_OVERRUN");
		}
		if (errors & CE_RXOVER)
		{
			Win32Log("[Win32WriteByteToComPort] Comm Error: CE_RXOVER");
		}
		if (errors & CE_RXPARITY)
		{
			Win32Log("[Win32WriteByteToComPort] Comm Error: CE_RXPARITY");
		}
	}

	return success;
}