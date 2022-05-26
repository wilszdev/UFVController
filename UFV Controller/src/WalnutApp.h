#pragma once

struct ufv_state
{
	int drive;
	int panAngle;
	int tiltAngle;
	bool pumpOn;
	int motorPower;
};

extern ufv_state g_ufvState;

#define NUM_PRESETS 6
struct angle_preset
{
	struct
	{
		int pan;
		int tilt;
	};
	union
	{
		int angles[2];
	};
};

extern angle_preset g_anglePresets[NUM_PRESETS];


#define ANGLE_DEBOUNCE_DELAY_MS 100

void ApplyPreset(int index);