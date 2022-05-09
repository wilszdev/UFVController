#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"

#include "Win32Helpers.h"
#include "XInputHelpers.h"

#include <chrono>

#define DELAY_MS 100

#define NUM_PRESETS 6
const int PRESET_TILT_ANGLE_VALUES[NUM_PRESETS] = { -45,45,-45,45,-45,45 };

struct ufv_state
{
	int drive;
	int panAngle;
	int tiltAngle;
	bool pumpOn;
};

static ufv_state ufvState = {};

class DriveLayer : public Walnut::Layer
{
public:
	virtual void OnUIRender() override
	{
		ImGui::Begin("Drive");
		ImGui::SliderInt("##it's about drive, it's about power", &ufvState.drive, -1, 1, ufvState.drive == 0 ? "Brake" : (ufvState.drive == 1 ? "Forward" : "Reverse"));
		ImGui::End();
	}
};

class FluidLayer : public Walnut::Layer
{
public:
	virtual void OnUIRender() override
	{
		ImGui::Begin("Fluid Delivery");

		if (ImGui::Button(ufvState.pumpOn ? "Turn pump off" : "Turn pump on"))
			ufvState.pumpOn = !ufvState.pumpOn;

		ImGui::BeginChild("##AnglesAndPresets", { 0, 225 }, true);

		{
			ImGui::Text("Tilt Angle"); ImGui::SameLine(100);
			ImGui::SliderInt("##TILT", &ufvState.tiltAngle, -5, 80);

			ImGui::Dummy({ 0, 0 }); ImGui::SameLine(100);

			if (ImGui::Button("Min##TILT", { 100, 0 }))
				ufvState.tiltAngle = -5;
			ImGui::SameLine();

			if (ImGui::Button("Zero##TILT", { 100, 0 }))
				ufvState.tiltAngle = 0;
			ImGui::SameLine();

			if (ImGui::Button("Max##TILT", { 100, 0 }))
				ufvState.tiltAngle = 80;

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			ImGui::Text("Pan Angle"); ImGui::SameLine(100);
			ImGui::SliderInt("##PAN", &ufvState.panAngle, -80, 80);

			ImGui::Dummy({ 0, 0 }); ImGui::SameLine(100);

			if (ImGui::Button("Min##PAN", { 100, 0 }))
				ufvState.panAngle = -80;
			ImGui::SameLine();

			if (ImGui::Button("Zero##PAN", { 100, 0 }))
				ufvState.panAngle = 0;
			ImGui::SameLine();

			if (ImGui::Button("Max##PAN", { 100, 0 }))
				ufvState.panAngle = 80;

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			ImGui::Text("Tilt Angle Presets (click to apply)");
			ImGui::Spacing();
			char buf[32] = {};
			for (int i = 0; i < NUM_PRESETS; ++i)
			{
				*buf = 0;
				sprintf_s(buf, 32, "Preset %d", i + 1);
				if (ImGui::Button(buf))
				{
					ufvState.tiltAngle = PRESET_TILT_ANGLE_VALUES[i];
					// m_power = m_presetPowerValues[i];
				}
				if (i != NUM_PRESETS - 1)
					ImGui::SameLine();
			}
		}
		
		ImGui::EndChild();

		ImGui::End();
	}
};

class GamepadControlsLayer : public Walnut::Layer
{
public:
	virtual void OnUIRender() override
	{
		ImGui::Begin("Gamepad Controls");

		if (ImGui::BeginTable("ctrltbl", 2, 0))
		{
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Button");
				ImGui::Dummy({ 0,5 });
				ImGui::Separator();
				ImGui::Dummy({ 0,5 });

				ImGui::TableNextColumn();
				ImGui::Text("Function");
				ImGui::Dummy({ 0,5 });
				ImGui::Separator();
			}

			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("RT");
				ImGui::Text("LT");
				ImGui::Dummy({ 0,5 });
				ImGui::Separator();
				ImGui::Dummy({ 0,5 });

				ImGui::TableNextColumn();
				ImGui::Text("forward (hold)");
				ImGui::Text("reverse (hold)");
				ImGui::Dummy({ 0,5 });
				ImGui::Separator();
			}

			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("DPAD LEFT");
				ImGui::Text("DPAD RIGHT");
				ImGui::Text("DPAD UP");
				ImGui::Dummy({ 0,5 });
				ImGui::Separator();
				ImGui::Dummy({ 0,5 });

				ImGui::TableNextColumn();
				ImGui::Text("previous preset");
				ImGui::Text("next preset");
				ImGui::Text("apply preset");
				ImGui::Dummy({ 0,5 });
				ImGui::Separator();
			}

			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("RS (X)");
				ImGui::Text("RS (Y)");
				ImGui::Text("A");
				ImGui::Dummy({ 0,5 });
				ImGui::Separator();
				ImGui::Dummy({ 0,5 });

				ImGui::TableNextColumn();
				ImGui::Text("adjust nozzle pan");
				ImGui::Text("adjust nozzle tilt");
				ImGui::Text("pump on (hold)");
				ImGui::Dummy({ 0,5 });
				ImGui::Separator();
			}

			ImGui::EndTable();
		}

		ImGui::End();
	}
};

template<typename T>
class RingBuffer
{
public:
	int m_head = 0;
	int m_tail = 0;
	int m_size = 0;
	T* m_data = nullptr;
public:
	RingBuffer() : RingBuffer(16)
	{
	}
	RingBuffer(int size)
	{
		m_size = size;
		m_data = new T[size];
	}
	~RingBuffer()
	{
		delete[] m_data;
		m_data = nullptr;
	}
	void Write(const T& value)
	{
		m_data[head] = value;
		m_head = (m_head + 1) % RINGBUF_SIZE;
		if (m_tail == m_head) // full
			m_tail = (m_tail + 1) % RINGBUF_SIZE;
	}
	void Write(const T&& value)
	{
		m_data[m_head] = value;
		m_head = (m_head + 1) % m_size;
		if (m_tail == m_head) // full
			m_tail = (m_tail + 1) % m_size;
	}
};

class CommsLayer : public Walnut::Layer
{
#define RECV_SIZE 0x4000
private:
	ufv_state m_oldState = {};
	bool m_spamData = false;
	int m_spamCount = 0;
	int m_bufferedByte = -1;
	RingBuffer<char> m_outgoingData{ 32 };
	std::chrono::steady_clock::time_point m_lastTilt;
	std::chrono::steady_clock::time_point m_lastPan;
	bool m_skippedTiltAngleDueToDebounce = false;
	bool m_skippedPanAngleDueToDebounce = false;
	char* m_recvBuffer = nullptr;
	RingBuffer<std::string> m_incomingData{ 24 };
	HANDLE m_comPort = 0;
	char m_comPortBuffer[8] = {};
private:
	void DoOutgoingComms()
	{
		if (!m_comPort) return;

		if (m_spamData)
		{
			if (Win32WriteByteToComPort(m_comPort, (char)0xAB))
			{
				m_outgoingData.Write((char)0xAB);
				++m_spamCount;
			}
			else
			{
				Win32Log("[SPAM DATA] Spam failed after %d bytes were written.", m_spamCount);
				m_spamData = false;
			}
		}

		if (m_bufferedByte != -1)
			if (Win32WriteByteToComPort(m_comPort, m_bufferedByte & 0xFF))
				m_bufferedByte = -1;

		if (m_bufferedByte == -1)
		{
			if (ufvState.drive != m_oldState.drive)
			{
				switch (ufvState.drive)
				{
				case 0:
				{
					if (Win32WriteByteToComPort(m_comPort, 'X'))
						m_outgoingData.Write('X');
					break;
				}
				case 1:
				{
					if (Win32WriteByteToComPort(m_comPort, 'F'))
						m_outgoingData.Write('F');
					break;
				}
				case -1:
				{
					if (Win32WriteByteToComPort(m_comPort, 'B'))
						m_outgoingData.Write('B');
					break;
				}
				default:
					break;
				}
			}

			if (ufvState.tiltAngle != m_oldState.tiltAngle)
			{
				std::chrono::nanoseconds diff = m_lastTilt - std::chrono::steady_clock::now();
				size_t ms = diff.count() / 1000000;

				if (ms > DELAY_MS)
				{
					if (Win32WriteByteToComPort(m_comPort, 'T'))
					{
						m_outgoingData.Write('T');
						if (Win32WriteByteToComPort(m_comPort, (char)ufvState.tiltAngle))
							m_outgoingData.Write((char)ufvState.tiltAngle);
						else
							m_bufferedByte = (int)(ufvState.tiltAngle & 0xFF);

						m_lastTilt = std::chrono::steady_clock::now();
					}
				}
				else
					m_skippedTiltAngleDueToDebounce = true;
			}

			if (ufvState.panAngle != m_oldState.panAngle)
			{
				std::chrono::nanoseconds diff = m_lastPan - std::chrono::steady_clock::now();
				size_t ms = diff.count() / 1000000;

				if (ms > DELAY_MS)
				{
					if (Win32WriteByteToComPort(m_comPort, 'A'))
					{
						m_outgoingData.Write('A');
						if (Win32WriteByteToComPort(m_comPort, (char)ufvState.panAngle))
							m_outgoingData.Write((char)ufvState.panAngle);
						else
							m_bufferedByte = (int)(ufvState.panAngle & 0xFF);

						m_lastPan = std::chrono::steady_clock::now();
					}
				}
				else
					m_skippedPanAngleDueToDebounce = true;
			}

			if (ufvState.pumpOn != m_oldState.pumpOn)
			{
				if (ufvState.pumpOn)
				{
					if (Win32WriteByteToComPort(m_comPort, 'P'))
						m_outgoingData.Write('P');
				}
				else
				{
					if (Win32WriteByteToComPort(m_comPort, 'N'))
						m_outgoingData.Write('N');
				}
			}
		}
	}

	void DoIncomingComms()
	{
		if (!m_comPort) return;

		memset(m_recvBuffer, 0, RECV_SIZE);

		size_t totalRead = 0;

		DWORD bytesRead = 0;
		do
		{
			if (!ReadFile(m_comPort, m_recvBuffer + totalRead, RECV_SIZE - 1 - totalRead, &bytesRead, 0))
				Win32Log("[DoIncomingComms] ReadFile() failed with error %d: %s", GetLastError(), Win32GetErrorCodeDescription(GetLastError()).c_str());
			totalRead += bytesRead;
		} while (bytesRead);
		
		if (totalRead && *m_recvBuffer)
			m_incomingData.Write(std::string{ m_recvBuffer });
	}
public:
	virtual void OnUIRender() override
	{
		DoIncomingComms();
		DoOutgoingComms();

		ImGui::Begin("Comms Layer");

		if (!m_comPort)
		{
			ImGui::Text("COM Port");
			ImGui::InputText("##adsf", m_comPortBuffer, 8); ImGui::SameLine();

			if (ImGui::Button("Open") && *m_comPortBuffer)
				m_comPort = Win32OpenAndConfigureComPort(m_comPortBuffer);
		}
		else
		{
			ImGui::Text("Currently using %s", m_comPortBuffer); ImGui::SameLine();
			if (ImGui::Button("Close"))
			{
				CloseHandle(m_comPort);
				m_comPort = 0;
			}
			ImGui::SameLine();

			if (ImGui::Button("Reopen"))
			{
				CloseHandle(m_comPort);
				m_comPort = 0;
				m_comPort = Win32OpenAndConfigureComPort(m_comPortBuffer);
			}

			if (ImGui::Checkbox("spam", &m_spamData))
				m_spamCount = 0;
			ImGui::SameLine();
			ImGui::Text("SpamCount: %d", m_spamCount);
		}

		ImGui::BeginTabBar("asdfasdfasdf");
		if (ImGui::BeginTabItem("Incoming Comms"))
		{
			for (int i = m_incomingData.m_tail;
				i < ((m_incomingData.m_head + 1 == m_incomingData.m_tail)
					? m_incomingData.m_tail + m_incomingData.m_size - 1
					: m_incomingData.m_tail + m_incomingData.m_head);
				++i)
			{
				const std::string& str = m_incomingData.m_data[i % m_incomingData.m_size];
				ImGui::Text(str.c_str());
			}

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Outgoing Comms"))
		{

			if (ImGui::BeginTable("outgoing ringbuffer", 4))
			{
				char buf[32] = {};
				for (int i = m_outgoingData.m_tail;
					i < ((m_outgoingData.m_head + 1 == m_outgoingData.m_tail)
						? m_outgoingData.m_tail + m_outgoingData.m_size - 1
						: m_outgoingData.m_tail + m_outgoingData.m_head); ++i)
				{
					*buf = 0;
					const char& c = m_outgoingData.m_data[i % m_outgoingData.m_size];
					bool printable = (c > ' ' && c < '~');
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);

					sprintf_s(buf, "'%c'", printable ? c & 0xff : '-');
					ImGui::Text(buf);
					ImGui::TableNextColumn();

					sprintf_s(buf, "%d", (unsigned int)(c & 0xff));
					ImGui::Text(buf);
					ImGui::TableNextColumn();

					sprintf_s(buf, "%d", (int)(signed char)(c & 0xff));
					ImGui::Text(buf);
					ImGui::TableNextColumn();

					sprintf_s(buf, "0x%02x", c & 0xff);
					ImGui::Text(buf);
				}

				ImGui::EndTable();
			}

			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();

		ImGui::End();

		int tmpP = m_oldState.panAngle;
		int tmpT = m_oldState.tiltAngle;

		memcpy(&m_oldState, &ufvState, sizeof(ufv_state));

		if (m_skippedPanAngleDueToDebounce)
			m_oldState.panAngle = tmpP;
		if (m_skippedTiltAngleDueToDebounce)
			m_oldState.tiltAngle = tmpT;
	}

	virtual void OnAttach()
	{
		m_recvBuffer = new char[RECV_SIZE];
	}

	virtual void OnDetach() override
	{
		if (m_recvBuffer)
		{
			delete[] m_recvBuffer;
			m_recvBuffer = nullptr;
		}
		if (m_comPort)
		{
			CloseHandle(m_comPort);
			m_comPort = 0;
		}
	}
};

class InputLayer : public Walnut::Layer
{
private:
	float m_angleSensitivityX = -2.5f;
	float m_angleSensitivityY = 2.5f;
	float m_powerSensitivity = 5.0f;
	int m_presetIndex = 0;

	controller_input m_input[2] = { 0 };
	controller_input* m_newController = &m_input[0];
	controller_input* m_oldController = &m_input[1];
public:
	virtual void OnAttach() override
	{
		Win32XInputLoad();
	}

	virtual void OnUIRender() override
	{
		// poll input
		PollXInput();

		// render imgui
		ImGui::Begin("Input Layer");

		ImGui::Text("Current Preset Selected: %d (DPAD UP to apply)", m_presetIndex + 1);
		ImGui::Dummy({ 0, 5 });

		ImGui::Text("Sensitivity Parameters");
		ImGui::SliderFloat("angleX",  & m_angleSensitivityX, -10.0f, 10.0f);
		ImGui::SliderFloat("angleY",  & m_angleSensitivityY, -10.0f, 10.0f);
		ImGui::SliderFloat("power",  & m_powerSensitivity, -10.0f, 10.0f);
		ImGui::Dummy({ 0, 5 });

		ImGui::Text("Controller Input State");
		{
			ImGui::BeginTable("inputs", 3);

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

		ImGui::End();

		ActOnInput();
	}
private:
	void PollXInput()
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

	void ActOnInput()
	{
		if (m_newController->A.pressed && !ufvState.pumpOn)
			ufvState.pumpOn = true;

		if (m_newController->A.released && ufvState.pumpOn)
			ufvState.pumpOn = false;

		if (ufvState.drive == 0 && m_newController->rightTriggerDigital.pressed && !m_newController->leftTriggerDigital.pressed)
			ufvState.drive = 1;
		if (ufvState.drive == 1 && m_newController->rightTriggerDigital.released)
			ufvState.drive = 0;

		if (ufvState.drive == 0 && m_newController->leftTriggerDigital.pressed && !m_newController->rightTriggerDigital.pressed)
			ufvState.drive = -1;
		if (ufvState.drive == -1 && m_newController->leftTriggerDigital.released)
			ufvState.drive = 0;

		if (m_newController->rightStick.avgX != 0.0)
		{
			float diff = m_newController->rightStick.avgX * m_angleSensitivityX;
			int truncDiff = (int)diff;
			
			ufvState.panAngle += truncDiff;
			if (ufvState.panAngle > 80) ufvState.panAngle = 80;
			else if (ufvState.panAngle < -80) ufvState.panAngle = -80;
		}

		if (m_newController->rightStick.avgY != 0.0)
		{
			float diff = m_newController->rightStick.avgY * m_angleSensitivityY;
			int truncDiff = (int)diff;

			ufvState.tiltAngle += truncDiff;
			if (ufvState.tiltAngle > 80) ufvState.tiltAngle = 80;
			else if (ufvState.tiltAngle < -5) ufvState.tiltAngle = -5;
		}

		if (m_newController->right.pressed)
			m_presetIndex = (m_presetIndex + 1) % NUM_PRESETS;

		if (m_newController->left.pressed)
			m_presetIndex = (m_presetIndex + NUM_PRESETS - 1) % NUM_PRESETS;

		if (m_newController->up.pressed)
			ufvState.tiltAngle = PRESET_TILT_ANGLE_VALUES[m_presetIndex];
	}
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "UFV Controller";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<InputLayer>();
	app->PushLayer<DriveLayer>();
	app->PushLayer<FluidLayer>();
	app->PushLayer<GamepadControlsLayer>();
	app->PushLayer<CommsLayer>();
	app->SetMenubarCallback([app]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				app->Close();
			}
			ImGui::EndMenu();
		}
	});
	return app;
}
