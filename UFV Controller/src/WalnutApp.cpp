#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"

#include "Win32Helpers.h"
#include "XInputHelpers.h"

struct ufv_state
{
	int drive;
	int pan;
	int tilt;
	bool pump;
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

		ImGui::Checkbox("Pump on", &ufvState.pump);

		ImGui::SliderInt("Tilt angle action", &ufvState.tilt, -1, 1, ufvState.tilt == 0 ? "None" : (ufvState.tilt == 1 ? "Up" : "Down"));

		ImGui::SliderInt("Pan angle action", &ufvState.pan, -1, 1, ufvState.pan == 0 ? "None" : (ufvState.pan == 1 ? "Right" : "Left"));

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

#define RINGBUF_SIZE 32

class OutgoingComLayer : public Walnut::Layer
{
private:
	HANDLE m_comPort = 0;
	char m_comPortBuffer[8] = {};
	ufv_state m_oldState = {};
	char m_outBuffer[RINGBUF_SIZE] = {};
	int m_head = 0;
	int m_tail = 0;
	int bufferedByte = -1;
private:
	void WriteToRingBuf(const char c)
	{
		m_outBuffer[m_head] = c;

		m_head = (m_head + 1) % RINGBUF_SIZE;
		if (m_tail == m_head) // full
			m_tail = (m_tail + 1) % RINGBUF_SIZE;
	}
public:
	virtual void OnUIRender() override
	{
		ImGui::Begin("Outgoing COM");

		if (!m_comPort)
		{
			ImGui::Text("Outgoing COM Port");
			ImGui::InputText("##adsf", m_comPortBuffer, 8); ImGui::SameLine();

			if (ImGui::Button("Attach") && *m_comPortBuffer)
				m_comPort = Win32OpenAndConfigureComPort(m_comPortBuffer);
		}
		else
		{
			ImGui::Text("Attached to %s", m_comPortBuffer); ImGui::SameLine();
			if (ImGui::Button("Detach"))
			{
				CloseHandle(m_comPort);
				m_comPort = 0;
			}
			ImGui::SameLine();

			if (ImGui::Button("Reattach"))
			{
				CloseHandle(m_comPort);
				m_comPort = 0;
				m_comPort = Win32OpenAndConfigureComPort(m_comPortBuffer);
			}
		}

		if (m_comPort)
		{
			if (ufvState.drive != m_oldState.drive)
			{
				switch (ufvState.drive)
				{
				case 0:
					{
						if (Win32WriteByteToComPort(m_comPort, 'X'))
							WriteToRingBuf('X');
						break;
					}
				case 1:
					{
						if (Win32WriteByteToComPort(m_comPort, 'F'))
							WriteToRingBuf('F');
						break;
					}
				case -1:
					{
						if (Win32WriteByteToComPort(m_comPort, 'B'))
							WriteToRingBuf('B');
						break;
					}
				default:
					break;
				}
			}

			switch (ufvState.tilt)
			{
			case 1:
				if (Win32WriteByteToComPort(m_comPort, 'U'))
					WriteToRingBuf('U');
				break;
			case -1:
				if (Win32WriteByteToComPort(m_comPort, 'D'))
					WriteToRingBuf('D');
				break;
			default:
				break;
			}

			switch (ufvState.pan)
			{
			case 1:
				if (Win32WriteByteToComPort(m_comPort, 'R'))
					WriteToRingBuf('R');
				break;
			case -1:
				if (Win32WriteByteToComPort(m_comPort, 'L'))
					WriteToRingBuf('L');
				break;
			default:
				break;
			}

			if (ufvState.pump != m_oldState.pump)
			{
				if (ufvState.pump)
				{

					if (Win32WriteByteToComPort(m_comPort, 'P'))
						WriteToRingBuf('P');
				}
				else
				{
						if (Win32WriteByteToComPort(m_comPort, 'N'))
							WriteToRingBuf('N');
				}
			}
		

			if (ImGui::BeginTable("outgoing ringbuffer", 4))
			{
				char buf[32] = {};
				for (int i = m_tail; i < ((m_head + 1 == m_tail) ? m_tail + RINGBUF_SIZE - 1: m_tail + m_head); ++i)
				{
					*buf = 0;
					char c = m_outBuffer[i % RINGBUF_SIZE];
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
		}

		ImGui::End();

		memcpy(&m_oldState, &ufvState, sizeof(ufv_state));
	}
};

class IncomingComLayer : public Walnut::Layer
{
private:
	HANDLE m_comPort = 0;
	char m_comPortBuffer[8] = {};
private:
	std::string m_outBuffer[RINGBUF_SIZE] = {};
	int m_head = 0;
	int m_tail = 0;
private:
	void WriteToRingBuf(const char* str)
	{
		m_outBuffer[m_head] = std::string{ str };

		m_head = (m_head + 1) % RINGBUF_SIZE;
		if (m_tail == m_head) // full
			m_tail = (m_tail + 1) % RINGBUF_SIZE;
	}
public:
	virtual void OnUIRender() override
	{
		ImGui::Begin("Incoming COM");

		if (!m_comPort)
		{
			ImGui::Text("Incoming COM Port");
			ImGui::InputText("##jkl;", m_comPortBuffer, 8); ImGui::SameLine();

			if (ImGui::Button("Attach") && *m_comPortBuffer)
			{
				// attach to com port
				m_comPort = Win32OpenAndConfigureComPort(m_comPortBuffer);

				// setup read timeouts, so we can synchronous reads don't block for long.
				if (m_comPort)
				{
					COMMTIMEOUTS ct = {};

					ct.ReadIntervalTimeout = 5;
					ct.ReadTotalTimeoutMultiplier = 0;
					ct.ReadTotalTimeoutConstant = 5;

					SetCommTimeouts(m_comPort, &ct);
				}
			}
		}

		if (m_comPort)
		{
			char buffer[128] = {};
			DWORD bytesRead = 0;
			if (ReadFile(m_comPort, buffer, 128, &bytesRead, 0))
			{
				if (bytesRead && *buffer)
					WriteToRingBuf(buffer);
			}
			else
			{
				Win32Log("[ReadFile] failed.");
			}

			// print everything
			for (int i = m_tail; i < ((m_head + 1 == m_tail) ? m_tail + RINGBUF_SIZE - 1 : m_tail + m_head); ++i)
				ImGui::Text(m_outBuffer[i % RINGBUF_SIZE].c_str());
		}

		ImGui::End();
	}
	virtual void OnDetach() override
	{
		CloseHandle(m_comPort);
		m_comPort = 0;
	}
};

class InputLayer : public Walnut::Layer
{
private:
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
		if (m_newController->A.pressed && !ufvState.pump)
			ufvState.pump = true;

		if (m_newController->A.released && ufvState.pump)
			ufvState.pump = false;

		if (ufvState.drive == 0 && m_newController->rightTriggerDigital.pressed && !m_newController->leftTriggerDigital.pressed)
			ufvState.drive = 1;
		if (ufvState.drive == 1 && m_newController->rightTriggerDigital.released)
			ufvState.drive = 0;

		if (ufvState.drive == 0 && m_newController->leftTriggerDigital.pressed && !m_newController->rightTriggerDigital.pressed)
			ufvState.drive = -1;
		if (ufvState.drive == -1 && m_newController->leftTriggerDigital.released)
			ufvState.drive = 0;

		if (m_newController->rightStick.avgX > 0.5)
			ufvState.pan = 1;
		else if (m_newController->rightStick.avgX < -0.5)
			ufvState.pan = -1;
		else
			ufvState.pan = 0;

		if (m_newController->rightStick.avgY > 0.5)
			ufvState.tilt = 1;
		else if (m_newController->rightStick.avgY < -0.5)
			ufvState.tilt = -1;
		else
			ufvState.tilt = 0;
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
	app->PushLayer<IncomingComLayer>();
	app->PushLayer<OutgoingComLayer>();
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
