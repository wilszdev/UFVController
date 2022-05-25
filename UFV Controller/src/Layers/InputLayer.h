#pragma once
#include "../WalnutApp.h"
#include "Walnut/Application.h"
#include "../XInputHelpers.h"

class InputLayer : public Walnut::Layer
{
private:
	float m_angleSensitivityX = -2.5f;
	float m_angleSensitivityY = 2.5f;
	float m_motorPowerSensitivity = 5.0f;
	int m_presetIndex = 0;

	controller_input m_input[2] = { 0 };
	controller_input* m_newController = &m_input[0];
	controller_input* m_oldController = &m_input[1];
public:
	virtual void OnAttach() override;
	virtual void OnUIRender() override;
private:
	void PollXInput();
	void ActOnInput();
};
