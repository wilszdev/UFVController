#pragma once
#include "GamepadControlsLayer.h"
#include "imgui.h"

void GamepadControlsLayer::OnUIRender()
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
			ImGui::Text("DPAD LEFT");
			ImGui::Text("DPAD RIGHT");
			ImGui::Text("DPAD UP");
			ImGui::Dummy({ 0,5 });
			ImGui::Separator();
			ImGui::Dummy({ 0,5 });

			ImGui::TableNextColumn();
			ImGui::Text("previous preset");
			ImGui::Text("next preset");
			ImGui::Text("apply preset");
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
