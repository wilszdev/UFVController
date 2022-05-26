#pragma once
#include "Walnut/Application.h"
#include "../WalnutApp.h"

class PresetsLayer : public Walnut::Layer
{
public:
	virtual void OnUIRender() override;
	virtual void OnAttach() override;
};