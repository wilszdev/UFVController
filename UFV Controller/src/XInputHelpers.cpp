#include "XInputHelpers.h"

X_INPUT_GET_STATE_FN(XInputGetStateStub) { return ERROR_DEVICE_NOT_CONNECTED; }
X_INPUT_SET_STATE_FN(XInputSetStateStub) { return ERROR_DEVICE_NOT_CONNECTED; }
x_input_get_state_fn* g_XInputGetState = XInputGetStateStub;
x_input_set_state_fn* g_XInputSetState = XInputSetStateStub;


void Win32XInputLoad()
{
	const char* names[] =
	{
		"xinput1_4.dll",
		"xinput1_3.dll",
		"xinput9_1_0.dll",
		"xinput1_2.dll",
		"xinput1_1.dll",
	};

	HMODULE xinput = 0;
	int i;
	for (i = 0; !xinput && i < sizeof(names) / sizeof(*names); ++i)
	{
		xinput = LoadLibraryA(names[i]);
	}
	
	if (xinput)
	{
		XInputGetState = (x_input_get_state_fn*)GetProcAddress(xinput, "XInputGetState");
		XInputSetState = (x_input_set_state_fn*)GetProcAddress(xinput, "XInputSetState");
		Win32Log("[Win32XInputLoad] Using %s", names[i - 1]);
	}
	else
	{
		Win32Log("[Win32XInputLoad] Could not find an xinput dll");
	}
}

void Win32XInputProcessDigitalButton(DWORD XInputButtonState,
	button_state* oldState, DWORD buttonMask,
	button_state* newState)
{
	newState->isDown = ((XInputButtonState & buttonMask) == buttonMask);
	newState->released = !newState->isDown && oldState->isDown;
	newState->pressed = newState->isDown && !oldState->isDown;
}

float Win32XInputProcessStickValue(SHORT stickValue, int deadzoneValue)
{
	float result = 0.0f;
	if (stickValue < -deadzoneValue)
	{
		result = (float)(stickValue + deadzoneValue) / (32768.0f - deadzoneValue);
	}
	else if (stickValue > deadzoneValue)
	{
		result = (float)(stickValue - deadzoneValue) / (32767.0f - deadzoneValue);
	}
	return result;
}

float Win32XInputProcessTriggerValue(BYTE triggerValue, int threshold)
{
	float result = 0.0f;
	if (triggerValue > threshold)
	{
		result = (float)(triggerValue - threshold) / (float)(255 - threshold);
	}
	return result;
}
