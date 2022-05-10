#include "CommsLayer.h"
#include "../Win32Helpers.h"

#define COMMS_RECV_BUFFER_SIZE 0x1000

void CommsLayer::DoOutgoingComms()
{
	if (!m_comPort) return;

	if (m_bufferedByte != -1)
	{
		++m_bufferedByteFailCount;
		if (m_bufferedByteFailCount > m_bufferedByteRetryCount ||
			Win32WriteByteToComPort(m_comPort, m_bufferedByte & 0xFF))
		{
			m_bufferedByte = -1;
			m_bufferedByteFailCount = 0;
		}
	}

	if (m_bufferedByte != -1) return; 

	if (g_ufvState.drive != m_oldState.drive)
	{
		switch (g_ufvState.drive)
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

	if (g_ufvState.tiltAngle != m_oldState.tiltAngle)
	{
		std::chrono::nanoseconds diff = m_lastTilt - std::chrono::steady_clock::now();
		size_t ms = diff.count() / 1000000;

		if (ms > ANGLE_DEBOUNCE_DELAY_MS)
		{
			if (Win32WriteByteToComPort(m_comPort, 'T'))
			{
				m_outgoingData.Write('T');
				if (Win32WriteByteToComPort(m_comPort, (char)g_ufvState.tiltAngle))
					m_outgoingData.Write((char)g_ufvState.tiltAngle);
				else
					m_bufferedByte = (int)(g_ufvState.tiltAngle & 0xFF);

				m_lastTilt = std::chrono::steady_clock::now();
			}
		}
		else
			m_skippedTiltAngleDueToDebounce = true;
	}

	if (g_ufvState.panAngle != m_oldState.panAngle)
	{
		std::chrono::nanoseconds diff = m_lastPan - std::chrono::steady_clock::now();
		size_t ms = diff.count() / 1000000;

		if (ms > ANGLE_DEBOUNCE_DELAY_MS)
		{
			if (Win32WriteByteToComPort(m_comPort, 'A'))
			{
				m_outgoingData.Write('A');
				if (Win32WriteByteToComPort(m_comPort, (char)g_ufvState.panAngle))
					m_outgoingData.Write((char)g_ufvState.panAngle);
				else
					m_bufferedByte = (int)(g_ufvState.panAngle & 0xFF);

				m_lastPan = std::chrono::steady_clock::now();
			}
		}
		else
			m_skippedPanAngleDueToDebounce = true;
	}

	if (g_ufvState.pumpOn != m_oldState.pumpOn)
	{
		if (g_ufvState.pumpOn)
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

	if (g_ufvState.burstDuration != m_oldState.burstDuration)
	{
		if (Win32WriteByteToComPort(m_comPort, 'S'))
		{
			m_outgoingData.Write('S');

			char lo = g_ufvState.burstDuration & 0xFF;
			char hi = (g_ufvState.burstDuration & 0xFF00) >> 8;

			if (Win32WriteByteToComPort(m_comPort, lo))
			{
				m_outgoingData.Write(lo);
				if (Win32WriteByteToComPort(m_comPort, hi))
					m_outgoingData.Write(hi);
			}
		}
	}

	if (g_ufvState.shouldBurst && !m_oldState.shouldBurst)
	{
		if (Win32WriteByteToComPort(m_comPort, 'Z'))
		{
			m_outgoingData.Write('Z');
			g_ufvState.shouldBurst = false;
		}
	}
}

void CommsLayer::DoIncomingComms()
{
	if (!m_comPort) return;

	memset(m_recvBuffer, 0, COMMS_RECV_BUFFER_SIZE);

	size_t totalRead = 0;

	DWORD bytesRead = 0;
	do
	{
		if (!ReadFile(m_comPort, m_recvBuffer + totalRead, COMMS_RECV_BUFFER_SIZE - 1 - totalRead, &bytesRead, 0))
			Win32Log("[DoIncomingComms] ReadFile() failed with error %d: %s", GetLastError(), Win32GetErrorCodeDescription(GetLastError()).c_str());
		totalRead += bytesRead;
	} while (bytesRead);

	if (totalRead && *m_recvBuffer)
		m_incomingData.Write(std::string{ m_recvBuffer });
}

void CommsLayer::OnUIRender()
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
	}

	ImGui::BeginTabBar("asdfasdfasdf");

	if (ImGui::BeginTabItem("Outgoing Comms"))
	{
		ImGui::Text("Buffered byte retry count"); ImGui::SameLine();

		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 16.0f);
			ImGui::TextUnformatted("If a write fails and is buffered, it will be retried this many times before the byte is discarded.");
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}

		ImGui::SliderInt("##bbfailthreshold", &m_bufferedByteRetryCount, 0, 25);

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

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

	ImGui::EndTabBar();

	ImGui::End();

	int tmpP = m_oldState.panAngle;
	int tmpT = m_oldState.tiltAngle;

	memcpy(&m_oldState, &g_ufvState, sizeof(ufv_state));

	if (m_skippedPanAngleDueToDebounce)
		m_oldState.panAngle = tmpP;
	if (m_skippedTiltAngleDueToDebounce)
		m_oldState.tiltAngle = tmpT;
}

void CommsLayer::OnAttach()
{
	m_recvBuffer = new char[COMMS_RECV_BUFFER_SIZE];
}

void CommsLayer::OnDetach()
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
