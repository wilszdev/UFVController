#include "FluidLayer.h"

#include "../WalnutApp.h"
#include "imgui.h"

#include <cstdio>

void FluidLayer::OnUIRender()
{
	ImGui::Begin("Fluid Delivery");

	ImGui::Checkbox("Pump on", &g_ufvState.pumpOn);

	ImGui::BeginChild("##AnglesAndPresets", { 0, 225 }, true);

	{
		ImGui::Text("Tilt Angle"); ImGui::SameLine(100);
		ImGui::SliderInt("##TILT", &g_ufvState.tiltAngle, -5, 80);

		ImGui::Dummy({ 0, 0 }); ImGui::SameLine(100);

		if (ImGui::Button("Min##TILT", { 100, 0 }))
			g_ufvState.tiltAngle = -5;
		ImGui::SameLine();

		if (ImGui::Button("Zero##TILT", { 100, 0 }))
			g_ufvState.tiltAngle = 0;
		ImGui::SameLine();

		if (ImGui::Button("Max##TILT", { 100, 0 }))
			g_ufvState.tiltAngle = 80;

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		ImGui::Text("Pan Angle"); ImGui::SameLine(100);
		ImGui::SliderInt("##PAN", &g_ufvState.panAngle, -80, 80);

		ImGui::Dummy({ 0, 0 }); ImGui::SameLine(100);

		if (ImGui::Button("Min##PAN", { 100, 0 }))
			g_ufvState.panAngle = -80;
		ImGui::SameLine();

		if (ImGui::Button("Zero##PAN", { 100, 0 }))
			g_ufvState.panAngle = 0;
		ImGui::SameLine();

		if (ImGui::Button("Max##PAN", { 100, 0 }))
			g_ufvState.panAngle = 80;

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
				ApplyPreset(i);

			if (i != NUM_PRESETS - 1)
				ImGui::SameLine();
		}
	}

	ImGui::EndChild();

	ImGui::End();
}
