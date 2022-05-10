#include "InputLayer.h"

void InputLayer::OnAttach()
{
	Win32XInputLoad();
}

void InputLayer::OnUIRender()
{
	// poll input
	PollXInput();

	// render imgui
	ImGui::Begin("Input Layer");

	ImGui::Text("Current Preset Selected: %d (DPAD UP to apply)", m_presetIndex + 1);
	ImGui::Dummy({ 0, 5 });

	ImGui::Text("Sensitivity Parameters");
	ImGui::SliderFloat("angleX", &m_angleSensitivityX, -10.0f, 10.0f);
	ImGui::SliderFloat("angleY", &m_angleSensitivityY, -10.0f, 10.0f);
	ImGui::SliderFloat("power", &m_powerSensitivity, -10.0f, 10.0f);
	ImGui::Dummy({ 0, 5 });

	ImGui::Text("Controller Input State");
	{
		if (ImGui::BeginTable("inputs", 3))
		{

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);

			ImGui::Text("lt: %.2f", m_newController->leftTrigger);
			ImGui::Text("rt: %.2f", m_newController->rightTrigger);

			ImGui::Dummy({ 0,5 });
			ImGui::TableNextColumn();

			ImGui::Text("lt (digital): %d", m_newController->leftTriggerDigital.isDown);
			ImGui::Text("rt (digital): %d", m_newController->rightTriggerDigital.isDown);

			ImGui::Dummy({ 0,5 });
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);

			ImGui::Text("ls x: %.2f", m_newController->leftStick.avgX);
			ImGui::Text("ls y: %.2f", m_newController->leftStick.avgY);

			ImGui::Dummy({ 0,5 });
			ImGui::TableNextColumn();

			ImGui::Text("rs x: %.2f", m_newController->rightStick.avgX);
			ImGui::Text("rs y: %.2f", m_newController->rightStick.avgY);

			ImGui::Dummy({ 0,5 });
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);

			ImGui::Text("up: %d", m_newController->up.isDown);
			ImGui::Text("down: %d", m_newController->down.isDown);
			ImGui::Text("left: %d", m_newController->left.isDown);
			ImGui::Text("right: %d", m_newController->right.isDown);

			ImGui::Dummy({ 0,5 });
			ImGui::TableNextColumn();

			ImGui::Text("A: %d", m_newController->A.isDown);
			ImGui::Text("B: %d", m_newController->B.isDown);
			ImGui::Text("X: %d", m_newController->X.isDown);
			ImGui::Text("Y: %d", m_newController->Y.isDown);

			ImGui::Dummy({ 0,5 });
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);

			ImGui::Text("lb: %d", m_newController->lb.isDown);
			ImGui::Text("rb: %d", m_newController->rb.isDown);

			ImGui::Dummy({ 0,5 });
			ImGui::TableNextColumn();

			ImGui::Text("back: %d", m_newController->back.isDown);
			ImGui::Text("start: %d", m_newController->start.isDown);

			ImGui::EndTable();
		}
	}

	ImGui::End();

	ActOnInput();
}

void InputLayer::PollXInput()
{
	memset(m_newController, 0, sizeof(controller_input));

	// todo: only poll connected controllers
	XINPUT_STATE state;
	if (XInputGetState(0, &state) == ERROR_SUCCESS)
	{
		// todo: see if state.dwPacketNumber increments too fast (polling too slow)
		XINPUT_GAMEPAD* gpad = &state.Gamepad;

		m_newController->isActive = true;

		// trigger input
		m_newController->leftTrigger = Win32XInputProcessTriggerValue(gpad->bLeftTrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD);

		m_newController->rightTrigger = Win32XInputProcessTriggerValue(gpad->bRightTrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD);

		Win32XInputProcessDigitalButton(m_newController->leftTrigger > 0.0 ? 1 : 0,
			&m_oldController->leftTriggerDigital, 1,
			&m_newController->leftTriggerDigital);

		Win32XInputProcessDigitalButton(m_newController->rightTrigger > 0.0 ? 1 : 0,
			&m_oldController->rightTriggerDigital, 1,
			&m_newController->rightTriggerDigital);

		// stick input
		m_newController->leftStick.avgX = Win32XInputProcessStickValue(gpad->sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);

		m_newController->leftStick.avgY = Win32XInputProcessStickValue(gpad->sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);

		m_newController->rightStick.avgX = Win32XInputProcessStickValue(gpad->sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);

		m_newController->rightStick.avgY = Win32XInputProcessStickValue(gpad->sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);

		// digital (button) input
		Win32XInputProcessDigitalButton(gpad->wButtons,
			&m_oldController->back, XINPUT_GAMEPAD_BACK,
			&m_newController->back);

		Win32XInputProcessDigitalButton(gpad->wButtons,
			&m_oldController->start, XINPUT_GAMEPAD_START,
			&m_newController->start);

		Win32XInputProcessDigitalButton(gpad->wButtons,
			&m_oldController->A, XINPUT_GAMEPAD_A,
			&m_newController->A);

		Win32XInputProcessDigitalButton(gpad->wButtons,
			&m_oldController->B, XINPUT_GAMEPAD_B,
			&m_newController->B);

		Win32XInputProcessDigitalButton(gpad->wButtons,
			&m_oldController->X, XINPUT_GAMEPAD_X,
			&m_newController->X);

		Win32XInputProcessDigitalButton(gpad->wButtons,
			&m_oldController->Y, XINPUT_GAMEPAD_Y,
			&m_newController->Y);

		Win32XInputProcessDigitalButton(gpad->wButtons,
			&m_oldController->up, XINPUT_GAMEPAD_DPAD_UP,
			&m_newController->up);

		Win32XInputProcessDigitalButton(gpad->wButtons,
			&m_oldController->down, XINPUT_GAMEPAD_DPAD_DOWN,
			&m_newController->down);

		Win32XInputProcessDigitalButton(gpad->wButtons,
			&m_oldController->left, XINPUT_GAMEPAD_DPAD_LEFT,
			&m_newController->left);

		Win32XInputProcessDigitalButton(gpad->wButtons,
			&m_oldController->right, XINPUT_GAMEPAD_DPAD_RIGHT,
			&m_newController->right);

		Win32XInputProcessDigitalButton(gpad->wButtons,
			&m_oldController->lb, XINPUT_GAMEPAD_LEFT_SHOULDER,
			&m_newController->lb);

		Win32XInputProcessDigitalButton(gpad->wButtons,
			&m_oldController->rb, XINPUT_GAMEPAD_RIGHT_SHOULDER,
			&m_newController->rb);
	}

	controller_input* tmp = m_oldController;
	m_oldController = m_newController;
	m_newController = tmp;
}

void InputLayer::ActOnInput()
{
	if (m_newController->A.pressed && !g_ufvState.pumpOn)
		g_ufvState.pumpOn = true;

	if (m_newController->A.released && g_ufvState.pumpOn)
		g_ufvState.pumpOn = false;

	if (m_newController->X.pressed && !g_ufvState.pumpOn)
		g_ufvState.shouldBurst = true;

	if (g_ufvState.drive == 0 && m_newController->rightTriggerDigital.pressed && !m_newController->leftTriggerDigital.pressed)
		g_ufvState.drive = 1;
	if (g_ufvState.drive == 1 && m_newController->rightTriggerDigital.released)
		g_ufvState.drive = 0;

	if (g_ufvState.drive == 0 && m_newController->leftTriggerDigital.pressed && !m_newController->rightTriggerDigital.pressed)
		g_ufvState.drive = -1;
	if (g_ufvState.drive == -1 && m_newController->leftTriggerDigital.released)
		g_ufvState.drive = 0;

	if (m_newController->rightStick.avgX != 0.0)
	{
		float diff = m_newController->rightStick.avgX * m_angleSensitivityX;
		int truncDiff = (int)diff;

		g_ufvState.panAngle += truncDiff;
		if (g_ufvState.panAngle > 80) g_ufvState.panAngle = 80;
		else if (g_ufvState.panAngle < -80) g_ufvState.panAngle = -80;
	}

	if (m_newController->rightStick.avgY != 0.0)
	{
		float diff = m_newController->rightStick.avgY * m_angleSensitivityY;
		int truncDiff = (int)diff;

		g_ufvState.tiltAngle += truncDiff;
		if (g_ufvState.tiltAngle > 80) g_ufvState.tiltAngle = 80;
		else if (g_ufvState.tiltAngle < -5) g_ufvState.tiltAngle = -5;
	}

	if (m_newController->right.pressed)
		m_presetIndex = (m_presetIndex + 1) % NUM_PRESETS;

	if (m_newController->left.pressed)
		m_presetIndex = (m_presetIndex + NUM_PRESETS - 1) % NUM_PRESETS;

	if (m_newController->up.pressed)
		ApplyPreset(m_presetIndex);
}
