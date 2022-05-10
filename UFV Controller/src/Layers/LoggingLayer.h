#pragma once
#include "Walnut/Layer.h"

#include <string>
#include <vector>

class LoggingLayer : public Walnut::Layer
{
private:
	std::vector<std::string> m_loggedStrings{};
	bool m_shouldScroll = false;
public:
	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnUIRender() override;
private:
	static void Callback(const char* str, void* parameter);
};