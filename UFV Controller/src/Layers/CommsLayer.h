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
	void DoOutgoingComms();
	void DoIncomingComms();
public:
	virtual void OnUIRender() override;
	virtual void OnAttach() override;
	virtual void OnDetach() override;
 };