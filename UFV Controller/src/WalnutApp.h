#pragma once

struct ufv_state
{
	int drive;
	int panAngle;
	int tiltAngle;
	bool pumpOn;
};

extern ufv_state g_ufvState;

#define ANGLE_DEBOUNCE_DELAY_MS 100

#define NUM_PRESETS 6
const int PRESET_TILT_ANGLE_VALUES[NUM_PRESETS] = { -5,45,-5,45,-5,45 };
const int PRESET_PAN_ANGLE_VALUES[NUM_PRESETS] = { -60,60,-60,60,-60,60 };

void ApplyPreset(int index);
