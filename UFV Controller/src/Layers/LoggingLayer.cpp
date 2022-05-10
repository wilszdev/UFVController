#include "LoggingLayer.h"
#include "../Win32Helpers.h"

#include "imgui.h"

void LoggingLayer::OnAttach()
{
	if (g_logCallback == nullptr && g_logCallbackParameter == nullptr)
	{
		g_logCallback = Callback;
		g_logCallbackParameter = this;
	}
}

void LoggingLayer::OnDetach()
{
	if (g_logCallbackParameter == this)
	{
		g_logCallback = nullptr;
		g_logCallbackParameter = nullptr;
	}
}

void LoggingLayer::OnUIRender()
{
	ImGui::Begin("Log");

	for (const auto& s : m_loggedStrings)
		ImGui::Text(s.c_str());

	if (m_shouldScroll)
	{
		ImGui::SetScrollHereY(1.0f);
		m_shouldScroll = false;
	}

	ImGui::End();
}

void LoggingLayer::Callback(const char* str, void* parameter)
{
#define this _this
	LoggingLayer* this = reinterpret_cast<LoggingLayer*>(parameter);
	this->m_loggedStrings.emplace_back(str);
	this->m_shouldScroll = true;
#undef this
}
