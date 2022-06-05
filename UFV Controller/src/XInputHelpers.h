#pragma once
#include "Win32Helpers.h"
#include <Xinput.h>
#include <cstdint>

// XInputGetState
#define X_INPUT_GET_STATE_FN(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE* pState)
typedef X_INPUT_GET_STATE_FN(x_input_get_state_fn);
X_INPUT_GET_STATE_FN(XInputGetStateStub);
extern x_input_get_state_fn* g_XInputGetState;
#define XInputGetState g_XInputGetState

// XInputSetState
#define X_INPUT_SET_STATE_FN(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration)
typedef X_INPUT_SET_STATE_FN(x_input_set_state_fn);
X_INPUT_SET_STATE_FN(XInputSetStateStub);
extern x_input_set_state_fn* g_XInputSetState;
#define XInputSetState g_XInputSetState

void Win32XInputLoad();

struct stick_state
{
	float avgX;
	float avgY;
};

struct button_state
{
	// state of button
	bool isDown;
	// press/release events: combination of current and previous state
	bool released;
	bool pressed;
};

struct controller_input
{
	uint32_t isActive;

	union
	{
		stick_state sticks[2];
		struct
		{
			stick_state leftStick;
			stick_state rightStick;
		};
	};

	union
	{
		float triggers[2];
		struct
		{
			float leftTrigger;
			float rightTrigger;
		};
	};

	union
	{
		button_state buttons[14];
		struct
		{
			button_state up;
			button_state down;
			button_state left;
			button_state right;

			button_state A;
			button_state B;
			button_state X;
			button_state Y;

			button_state lb;
			button_state rb;

			button_state back;
			button_state start;

			button_state leftTriggerDigital;
			button_state rightTriggerDigital;

			// note: all buttons must be added above this line
			// for array size check. fake button.
			button_state terminator;
		};
	};
};

void Win32XInputProcessDigitalButton(DWORD XInputButtonState,
	button_state* oldState, DWORD buttonMask,
	button_state* newState);

float Win32XInputProcessStickValue(SHORT stickValue, int deadzoneValue);

float Win32XInputProcessTriggerValue(BYTE triggerValue, int threshold);