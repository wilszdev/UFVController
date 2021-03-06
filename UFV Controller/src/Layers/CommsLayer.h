#pragma once
#include "../WalnutApp.h"
#include "../RingBuffer.h"
#include "../Win32Helpers.h"

#include "Walnut/Application.h"

#include <chrono>

class CommsLayer : public Walnut::Layer
{
private:
	ufv_state m_oldState = {};
	int m_bufferedByte = -1;
	int m_bufferedByteFailCount = 0;
	int m_bufferedByteRetryCount = 5;
	RingBuffer<char> m_outgoingData{ 8 };
	std::chrono::steady_clock::time_point m_lastTilt;
	std::chrono::steady_clock::time_point m_lastPan;
	std::chrono::steady_clock::time_point m_lastMotorPower;
	bool m_skippedTiltAngleDueToDebounce = false;
	bool m_skippedPanAngleDueToDebounce = false;
	bool m_skippedMotorPowerDueToDebounce = false;
	char* m_recvBuffer = nullptr;
	RingBuffer<std::string> m_incomingData{ 8 };
	HANDLE m_comPort = 0;
	char m_comPortBuffer[8] = {};
private:
	void DoOutgoingComms();
	void DoIncomingComms();
public:
	virtual void OnUIRender() override;
	virtual void OnAttach() override;
	virtual void OnDetach() override;
 };