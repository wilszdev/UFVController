#include "PresetsLayer.h"
#include "../Win32Helpers.h"

static void SavePresets()
{
	HANDLE file = CreateFileA("presets.dat", GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	if (file && file != INVALID_HANDLE_VALUE)
	{
		DWORD size = sizeof(*g_anglePresets) * NUM_PRESETS;
		DWORD written = 0;
		if (!WriteFile(file, g_anglePresets, size, &written, 0))
			Win32Log("[SavePresets] WriteFile() failed with error %d: %s",
				GetLastError(), Win32GetErrorCodeDescription(GetLastError()).c_str());
		CloseHandle(file);
	}
	else
		Win32Log("[LoadPresets] CreateFileA() failed with error %d: %s",
			GetLastError(), Win32GetErrorCodeDescription(GetLastError()).c_str());
}

static void LoadPresets()
{
	HANDLE file = CreateFileA("presets.dat", GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
	if (file && file != INVALID_HANDLE_VALUE)
	{
		DWORD size = sizeof(*g_anglePresets) * NUM_PRESETS;
		DWORD read = 0;
		if (!ReadFile(file, g_anglePresets, size, &read, 0))
			Win32Log("[LoadPresets] ReadFile failed with error %d: %s",
				GetLastError(), Win32GetErrorCodeDescription(GetLastError()).c_str());
		CloseHandle(file);
	}
	else
		Win32Log("[LoadPresets] CreateFileA() failed with error %d: %s",
			GetLastError(), Win32GetErrorCodeDescription(GetLastError()).c_str());
}

void PresetsLayer::OnUIRender()
{
	ImGui::Begin("Presets");

	for (int i = 0; i < NUM_PRESETS; ++i)
	{
		char buf[16] = {};
		sprintf(buf, "Preset %d", i + 1);

		char buf2[18] = "##";
		strcat_s(buf2, buf);

		ImGui::Text(buf); ImGui::SameLine();
		ImGui::InputInt2(buf2, g_anglePresets[i].angles);
	}

	if (ImGui::Button("Save these presets"))
	{
		SavePresets();
	}

	ImGui::End();
}

void PresetsLayer::OnAttach()
{
	LoadPresets();
}
